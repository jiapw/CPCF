#include "tinyhttpd_fileserv.h"
#include "../os/file_dir.h"
#include "../os/file_zip.h"


namespace inet
{


LPBYTE HttpServerFiles::_AddFile(const rt::String_Ref& path, UINT datalen, LPCSTR mime)
{
	int pathlen = (int)path.GetLength();
	_FileData* p = (_FileData*)_Malloc32AL(BYTE,sizeof(_FileData)-1+datalen+pathlen+1);
	if(p)
	{
		memcpy((LPSTR)p->data,path.Begin(),pathlen+1);
		p->uri = rt::String_Ref((LPSTR)p->data,pathlen);
		p->mime = mime?mime:TinyHttpd::_GetMIME(rt::String_Ref(path));
		p->datasize = datalen + pathlen + 1;

		{	t_NameSpace::iterator it = _NameSpace.find(p->uri);
			if(it != _NameSpace.end())
			{
				_SafeFree32AL(it->second);
				_NameSpace.erase(it);
			}
		}
		_NameSpace.insert(std::pair<rt::String_Ref,_FileData*>(p->uri, p));

		rt::Zero(p->data + pathlen + 1,datalen);
		return p->data + pathlen + 1;
	}
	else return nullptr;
}

void HttpServerFiles::AddFile(const rt::String_Ref& path, LPCVOID pdata, UINT datalen, LPCSTR mime)
{
	rt::BufferEx<BYTE>	out_e;
	if(_HttpDataConv && _HttpDataConv(path, pdata, datalen, out_e))
	{
		LPBYTE buf = _AddFile(path, (UINT)out_e.GetSize(), mime);
		if(buf)memcpy(buf,out_e.Begin(), out_e.GetSize());	
	}
	else
	{
		LPBYTE buf = _AddFile(path, datalen, mime);
		if(buf)memcpy(buf,pdata, datalen);
	}
}

UINT HttpServerFiles::ImportZipFile(LPCSTR zip_file, LPCSTR path_prefix, UINT fsize_max)
{
	os::FileZip zip;
	rt::String_Ref prefix(path_prefix);

	UINT ret = 0;

	if(prefix.GetLength() && (prefix.Last() == '\\' || prefix.Last() == '/'))
		prefix = prefix.TrimRight(1);

	if(zip.Open(zip_file, os::File::Normal_Read, false))
	{
		rt::Buffer<BYTE>	data;
		rt::String path;
		UINT co = zip.GetEntryCount();
		for(UINT i=0;i<co;i++)
		{
			UINT fsize = zip.GetFileSize(i);
			if(fsize>0 && fsize<fsize_max)
			{
				path = prefix + '/' + zip.GetFileName(i);
				data.SetSize(fsize);
				if(zip.ExtractFile(i, data))
				{
					ret ++;
					AddFile(path, data, (UINT)data.GetSize(), TinyHttpd::_GetMIME(path));
					_LOG(path);
				}
			}
		}
	}

	return ret;
}

HttpServerFiles::~HttpServerFiles()
{
	RemoveAllFiles();
}

HttpServerFiles::HttpServerFiles()
{
	_HttpDataConv = nullptr;
}

void HttpServerFiles::RemoveAllFiles()
{
	for(t_NameSpace::iterator p = _NameSpace.begin();p != _NameSpace.end(); p++)
		_SafeFree32AL(p->second);
	_NameSpace.clear();
}

void HttpServerFiles::SendResponse(HttpResponse& response, const rt::String_Ref& path)
{
	t_NameSpace::iterator p = _NameSpace.find(path);
	if(p!=_NameSpace.end())
	{
		response.Send(p->second->GetPayload(),p->second->GetPayloadSize(),p->second->mime.Begin());
	}
	else
	{	response.SendHttpError(404);
	}
}

bool HttpServerFiles::OnRequest(HttpResponse& response)
{
	rt::String_Ref path = response.GetLnPath(this);
	SendResponse(response, path);

	return true;
}


bool HttpVirtualPath::OnRequest(HttpResponse& resp)
{
	rt::String_Ref uri = resp.GetLnPath(this);
	LPBYTE buf = nullptr;
	UINT	fsz;

	ASSERT_NONRECURSIVE;
	thread_local rt::BufferEx<BYTE>	conv_data;
	conv_data.ShrinkSize(0);

	rt::String a;
	(_MappedPath + uri.TrimRight(1)).ToString(a);

	if(!uri.IsEmpty() && uri.Last() == '/' && os::File::IsDirectory(a))
	{
		if(resp.HttpVerb == HTTP_GET)
		{
			os::FileList	flist;
			flist.Populate(a, nullptr, os::FileList::FLAG_SKIPHIDDEN);

			resp.SendChuncked_Begin(TinyHttpd::_MIMEs[TinyHttpd::MIME_HTML], _MaxAge);
			{
				resp.SendChuncked(__SS(
					<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
					<html xmlns="http://www.w3.org/1999/xhtml">
					<head><meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
					<title>Directories and Files in&nbsp;
				));
				resp.SendChuncked(a);
				resp.SendChuncked(__SS(
					</title>
					<style type="text/css">
					body,td,th { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; }
					body,td { background-color:#fff; padding-left:4px; }
					th { background-color:#DDD; font-weight:bold; }
					tr { height:18px; }
					td { background-color:#fff; padding-right: 8px; }
					a { color: #33339A; text-decoration: none; }
					</style></head><body><br />
				));

				if(!_ReadOnly)
				{	resp.SendChuncked(__SS(
						<form method="post" action="" enctype="multipart/form-data"><b>Upload a file:&nbsp;&nbsp;</b>
						<input type="file" name="file" onchange=" fn = this.value.replace(new RegExp('\\\\', 'g'), '/'); fn = './' + fn.substring(fn.lastIndexOf('/')+1); this.parentElement.action = './' + fn; this.parentElement.submit(); " />
						</form><br />
					));
				}

				resp.SendChuncked(__SS(<table border="0" cellpadding="0" cellspacing="2">));  // width="100%"

				if(uri.GetLength()>2)
				{
					resp.SendChuncked(
						rt::SS("<tr><td style='padding-right: 12px;'><a href=\"../\">../</a></td><td></td><td></td></tr>")
					);
				}

				// All folders
				for(UINT i=0;i<flist.GetCount();i++)
				{	
					if(flist.IsDirectory(i))
					{
						LPCSTR fn = flist.GetFullpath(i);
						__time64_t last_mod;
						os::File::GetPathTime(fn, NULL, NULL, &last_mod);					
						resp.SendChuncked(
							rt::SS("<tr><td style='padding-right: 12px;'><a href=\".") + flist.GetFilename(i) + rt::SS("/\">") + (flist.GetFilename(i).TrimLeft(1)) + rt::SS("/</a></td><td></td><td>") + 
							rt::tos::Timestamp<false>(last_mod*1000) +
							rt::SS("</td></tr>")
						);
					}
				}
				// All Files
				for(UINT i=0;i<flist.GetCount();i++)
				{	
					if(!flist.IsDirectory(i))
					{
						LPCSTR fn = flist.GetFullpath(i);
						ULONGLONG fsize = os::File::GetFileSize(fn);
						__time64_t last_mod;
						os::File::GetPathTime(fn, NULL, NULL, &last_mod);

						resp.SendChuncked(
							rt::SS("<tr><td style='padding-right: 12px;'><a href=\".") + flist.GetFilename(i) + rt::SS("\">") + (flist.GetFilename(i).TrimLeft(1)) + rt::SS("</a></td><td style='text-align:right;'>") + 
							rt::tos::FileSize<>(fsize) + rt::SS("</td><td>") + 
							rt::tos::Timestamp<false>(last_mod*1000) +
							rt::SS("</td></tr>")
						);
					}
				}

				resp.SendChuncked(__SS(</table></body></html>));
			}
			resp.SendChuncked_End();
		}
		else
		{	resp.SendHttpError(HTTP_NOT_ALLOWED);
		}

		return true;
	}
	else
	{
		if(resp.HttpVerb == HTTP_GET)
		{	
			a = _MappedPath + uri;
			if(os::File::IsDirectory(a))
			{	// index.html/index.htm
				rt::String q = a + rt::SS("/index.html");
				rt::String file;
				bool loaded;
				if(!(loaded = os::File::IsFile(q)))
				{
					q.SetLength(q.GetLength()-1);
					if(!(loaded = os::File::IsFile(q)))
					{	q = a + rt::SS("/default.html");
						if(!(loaded = os::File::IsFile(q)))
						{	
							q.SetLength(q.GetLength()-1);
							loaded = os::File::IsFile(q);
						}
					}
				}

				if(loaded)
				{
					a = resp.URI + q.TrimLeft(_MappedPath.GetLength());
					resp.SendRedirection(HTTP_FOUND, a, (int)a.GetLength());
				}
				else
					resp.SendHttpError(404);

				return true;
			}
			else
			{
				os::File	file;
				if(	file.Open(a) &&
					(fsz = (UINT)file.GetLength()) < MAX_FILELOAD_SIZE
				)
				{
					ULONGLONG offset = 0;
					UINT len = (UINT)fsz;

					if(resp.ParseRequestRange(fsz, &offset, &len))
					{
						if(	(buf = resp.GetWorkSpace(len)) &&
							file.Seek((SSIZE_T)offset) == offset &&
							file.Read(buf, len) == len
						)
						{	resp.Send(buf, len, TinyHttpd::_GetMIME(resp.URI), offset, offset+len-1, fsz);
							return true;
						}
					}
					else
					{
						if(	(buf = resp.GetWorkSpace(fsz)) &&
							file.Read(buf, fsz) == fsz
						)
						{	if(_HttpDataConv && _HttpDataConv(uri,buf,fsz,conv_data))
							{
								resp.Send(conv_data,(int)conv_data.GetSize(),TinyHttpd::_GetMIME(resp.URI),_MaxAge);
							}
							else
								resp.Send(buf,(int)fsz,TinyHttpd::_GetMIME(resp.URI),_MaxAge);

							return true;
						}
					}
				}
			}
		}
		else if(resp.HttpVerb == HTTP_POST)
		{	
			os::File file;
			if(resp.Body.GetLength())
			{	
				rt::String_Ref sep_line;
				if(	resp.Body.GetNextLine(sep_line) &&
					resp.Body.SubStr(resp.Body.GetLength()-4-sep_line.GetLength(), sep_line.GetLength()) == sep_line
				)
				{	LPCSTR p = sep_line.End();
					LPCSTR end = resp.Body.End() - sep_line.GetLength() - 4 - 2;
					for(;p<end;p++)
					{	if(*((DWORD*)p) == 0x0a0d0a0d)
							break;
					}

					p+=4;
					if(	p<end && 
						file.Open(_MappedPath + uri, os::File::Normal_Write, true) &&
						(file.Write(p, end - p) == (end - p))
					)
					{
						uri = resp.URI.GetDirectoryName();
						resp.SendRedirection(HTTP_MOVED, uri.Begin(), (int)uri.GetLength());
						return true;
					}
				}

				//_LOG(sep_line);
				//_LOG(resp.Body.SubStr(resp.Body.GetLength()-4-sep_line.GetLength(), sep_line.GetLength()));
			}

			resp.SendHttpError(HTTP_FORBIDDEN);
			return true;
		}
	}

	resp.SendHttpError(HTTP_NOT_FOUND);
	return true;
}

void HttpVirtualPath::SetMappedPath(LPCSTR p, bool readonly)
{
	_ReadOnly = readonly;
	os::File::ResolveRelativePath(p, _MappedPath);

	if(_MappedPath.Last() == '\\' || _MappedPath.Last() == '/')
		_MappedPath = rt::String_Ref(_MappedPath.Begin(),_MappedPath.GetLength()-1);

	if(!os::File::IsExist(_MappedPath))
		_LOG_WARNING("[HttpVirtualPath::SetMappedPath]: '"<<_MappedPath<<"' not exist");
}


HttpVirtualPath::HttpVirtualPath()
{
	_HttpDataConv = nullptr;
	_MaxAge = 0;
	_ReadOnly = true;
}









} // namespace inet

