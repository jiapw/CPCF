#include "../../core/rt/string_type.h"
#include "../../core/ext/ipp/ipp_image.h"
#include "../../core/ext/ipp/ipp_canvas.h"
#include "../../core/os/file_dir.h"
#include "../../core/os/kernel.h"
#include "../../core/ext/zlib/zlib.h"
#include "test.h"


void rt::UnitTests::ipp_canvas()
{
	ipp::Image_3c8u img;
	img.Load("test.jpg");

	ipp::Canvas_3c8u canvas(img);

	canvas.DrawPixel(110, 10, rt::Vec3b(255,0,0));
	canvas.SetPointSize(600);
	canvas.DrawPoint(200,100, rt::Vec4b(255,255,155,100));

	canvas.SetPointSize(4);
	canvas.DrawPoint(200,100, rt::Vec4b(0,200));
	canvas.DrawPoint(210.5f,100, rt::Vec4b(0,200));

	canvas.SetPointSize(3);
	canvas.DrawPoint(200,110, rt::Vec4b(0,200));
	canvas.DrawPoint(210.5f,110, rt::Vec4b(0,200));

	canvas.SetPointSize(2);
	canvas.DrawPoint(200,120, rt::Vec4b(0,200));
	canvas.DrawPoint(210.5f,120, rt::Vec4b(0,200));

	canvas.SetPointSize(1);
	canvas.DrawPoint(200,130, rt::Vec4b(0,200));
	canvas.DrawPoint(210.5f,130, rt::Vec4b(0,200));

	canvas.SetPointSize(0.5f);
	canvas.DrawPoint(200,140, rt::Vec4b(0,200));
	canvas.DrawPoint(210.5f,140, rt::Vec4b(0,200));

	canvas.SetPointSize(0.1f);
	canvas.DrawPoint(200,140, rt::Vec4b(0,200));
	canvas.DrawPoint(210.5f,140, rt::Vec4b(0,200));

	canvas.SetLineWidth(3);		canvas.DrawLine(100, 10, 200, 40, rt::Vec4b(0,200));
	canvas.SetLineWidth(2);		canvas.DrawLine(100, 20, 200, 50, rt::Vec4b(0,200));
	canvas.SetLineWidth(1);		canvas.DrawLine(100, 30, 200, 60, rt::Vec4b(0,200));
	canvas.SetLineWidth(0.5);	canvas.DrawLine(100, 40, 200, 70, rt::Vec4b(0,200));
	canvas.SetLineWidth(0.25);	canvas.DrawLine(100, 50, 200, 80, rt::Vec4b(0,200));

	canvas.SetLineWidth(30);	canvas.DrawLine(100+0.5f, 210+0.5f, 200+0.5f, 260+0.5f, rt::Vec4b(0,100));
								canvas.DrawLine(250+0.5f, 260+0.5f, 350+0.5f, 210+0.5f, rt::Vec4b(0,100));

	canvas.SetPointSize(1);
	canvas.DrawPoint(100, 210, rt::Vec3b(0, 255, 0));
	canvas.DrawPoint(200, 260, rt::Vec3b(0, 0, 255));
	canvas.DrawPoint(250, 260, rt::Vec3b(0, 255, 0));
	canvas.DrawPoint(350, 210, rt::Vec3b(0, 0, 255));

	canvas.SetLineWidth(3);		canvas.DrawLine(10, 100, 40, 200, rt::Vec4b(0,200));
	canvas.SetLineWidth(2);		canvas.DrawLine(20, 100, 50, 200, rt::Vec4b(0,200));
	canvas.SetLineWidth(1);		canvas.DrawLine(30, 100, 60, 200, rt::Vec4b(0,200));
	canvas.SetLineWidth(0.5);	canvas.DrawLine(40, 100, 70, 200, rt::Vec4b(0,200));
	canvas.SetLineWidth(0.25);	canvas.DrawLine(50, 100, 80, 200, rt::Vec4b(0,200));

	canvas.SetLineWidth(30);	canvas.DrawLine(210+0.5f, 100+0.5f, 260+0.5f, 200+0.5f, rt::Vec4b(0,100));
								canvas.DrawLine(260+0.5f, 250+0.5f, 210+0.5f, 350+0.5f, rt::Vec4b(0,100));

	UINT sz = 40;
	canvas.DrawImage(10,	 400,	 img.GetSub(560, 0, sz, sz));
	canvas.DrawImage(100.0f, 400.0f, img.GetSub(560, 0, sz, sz));
	canvas.DrawImage(199.5f, 400.0f, img.GetSub(560, 0, sz, sz));
	canvas.DrawImage(300.0f, 399.5f, img.GetSub(560, 0, sz, sz));
	canvas.DrawImage(399.5f, 399.5f, img.GetSub(560, 0, sz, sz));

	canvas.Save("draw.png", ipp::ImageCodec_PNG);	
}

LPCSTR rgb2label(const int* rgb, int* label)
{
	struct color_label
	{	LPCSTR name;
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	static const color_label cls[] = 
	{
	{"maroon",  128,0,0},
	{"dark red", 139,0,0},
	{"brown", 165,42,42},
	{"firebrick", 178,34,34},
	{"crimson", 220,20,60},
	{"red", 255,0,0},
	{"tomato", 255,99,71},
	{"coral", 255,127,80},
	{"indian red", 205,92,92},
	{"light coral", 240,128,128},
	{"dark salmon", 233,150,122},
	{"salmon", 250,128,114},
	{"light salmon", 255,160,122},
	{"orange red", 255,69,0},
	{"dark orange", 255,140,0},
	{"orange", 255,165,0},
	{"gold", 255,215,0},
	{"dark golden rod", 184,134,11},
	{"golden rod", 218,165,32},
	{"pale golden rod", 238,232,170},
	{"dark khaki", 189,183,107},
	{"khaki", 240,230,140},
	{"olive", 128,128,0},
	{"yellow", 255,255,0},
	{"yellow green", 154,205,50},
	{"dark olive green", 85,107,47},
	{"olive drab", 107,142,35},
	{"lawn green", 124,252,0},
	{"chart reuse", 127,255,0},
	{"green yellow", 173,255,47},
	{"dark green", 0,100,0},
	{"green", 0,128,0},
	{"forest green", 34,139,34},
	{"lime", 0,255,0},
	{"lime green", 50,205,50},
	{"light green", 144,238,144},
	{"pale green", 152,251,152},
	{"dark sea green", 143,188,143},
	{"medium spring green", 0,250,154},
	{"spring green", 0,255,127},
	{"sea green", 46,139,87},
	{"medium aqua marine", 102,205,170},
	{"medium sea green", 60,179,113},
	{"light sea green", 32,178,170},
	{"dark slate gray", 47,79,79},
	{"teal", 0,128,128},
	{"dark cyan", 0,139,139},
	{"aqua", 0,255,255},
	{"cyan", 0,255,255},
	{"light cyan", 224,255,255},
	{"dark turquoise", 0,206,209},
	{"turquoise", 64,224,208},
	{"medium turquoise", 72,209,204},
	{"pale turquoise", 175,238,238},
	{"aqua marine", 127,255,212},
	{"powder blue", 176,224,230},
	{"cadet blue", 95,158,160},
	{"steel blue", 70,130,180},
	{"corn flower blue", 100,149,237},
	{"deep sky blue", 0,191,255},
	{"dodger blue", 30,144,255},
	{"light blue", 173,216,230},
	{"sky blue", 135,206,235},
	{"light sky blue", 135,206,250},
	{"midnight blue", 25,25,112},
	{"navy", 0,0,128},
	{"dark blue", 0,0,139},
	{"medium blue", 0,0,205},
	{"blue", 0,0,255},
	{"royal blue", 65,105,225},
	{"blue violet", 138,43,226},
	{"indigo", 75,0,130},
	{"dark slate blue", 72,61,139},
	{"slate blue", 106,90,205},
	{"medium slate blue", 123,104,238},
	{"medium purple", 147,112,219},
	{"dark magenta", 139,0,139},
	{"dark violet", 148,0,211},
	{"dark orchid", 153,50,204},
	{"medium orchid", 186,85,211},
	{"purple", 128,0,128},
	{"thistle", 216,191,216},
	{"plum", 221,160,221},
	{"violet", 238,130,238},
	{"magenta", 255,0,255},
	{"orchid", 218,112,214},
	{"medium violet red", 199,21,133},
	{"pale violet red", 219,112,147},
	{"deep pink", 255,20,147},
	{"hot pink", 255,105,180},
	{"light pink", 255,182,193},
	{"pink", 255,192,203},
	{"antique white", 250,235,215},
	{"beige", 245,245,220},
	{"bisque", 255,228,196},
	{"blanched almond", 255,235,205},
	{"wheat", 245,222,179},
	{"corn silk", 255,248,220},
	{"lemon chiffon", 255,250,205},
	{"light golden rod yellow", 250,250,210},
	{"light yellow", 255,255,224},
	{"saddle brown", 139,69,19},
	{"sienna", 160,82,45},
	{"chocolate", 210,105,30},
	{"peru", 205,133,63},
	{"sandy brown", 244,164,96},
	{"burly wood", 222,184,135},
	{"tan", 210,180,140},
	{"rosy brown", 188,143,143},
	{"moccasin", 255,228,181},
	{"navajo white", 255,222,173},
	{"peach puff", 255,218,185},
	{"misty rose", 255,228,225},
	{"lavender blush", 255,240,245},
	{"linen", 250,240,230},
	{"old lace", 253,245,230},
	{"papaya whip", 255,239,213},
	{"sea shell", 255,245,238},
	{"mint cream", 245,255,250},
	{"slate gray", 112,128,144},
	{"light slate gray", 119,136,153},
	{"light steel blue", 176,196,222},
	{"lavender", 230,230,250},
	{"floral white", 255,250,240},
	{"alice blue", 240,248,255},
	{"ghost white", 248,248,255},
	{"honeydew", 240,255,240},
	{"ivory", 255,255,240},
	{"azure", 240,255,255},
	{"snow", 255,250,250},
	{"black", 0,0,0},
	{"dim gray", 105,105,105},
	{"gray", 128,128,128},
	{"dark gray", 169,169,169},
	{"silver", 192,192,192},
	{"light gray", 211,211,211},
	{"gainsboro", 220,220,220},
	{"white smoke", 245,245,245},
	{"white", 255,255,255}
	};

	int dist = 100000000;
	int best_idx = 0;

	for(UINT i=0;i<sizeofArray(cls);i++)
	{
		int d = rt::Sqr(rgb[0] - cls[i].r) + 
				rt::Sqr(rgb[1] - cls[i].g) + 
				rt::Sqr(rgb[2] - cls[i].b);
		if(d<dist)
		{	best_idx = i;
			dist = d;
		}
	}

	*label = best_idx;
	return cls[best_idx].name;
}

void major_color(LPCVOID pRGB32, UINT w, UINT h, UINT step, std::string& json)
{
	struct color_stat
	{	UINT r,g,b,count;
		static int comp(LPCVOID a, LPCVOID b)
		{	return ((color_stat*)b)->count - ((color_stat*)a)->count;
		}
	};

	color_stat hist[18];
	color_stat tot;
	memset(hist, 0, sizeof(hist));
	memset(&tot, 0, sizeof(tot));
	static const int col_thres_rg = 86;
	static const int col_thres_b = 100;

	LPCBYTE scanline = ((LPCBYTE)pRGB32) + h*step/6;
	for(UINT y=h/6; y<5*h/6; y++, scanline+=step)
	{
		LPCBYTE px = scanline + w/6*4;
		for(UINT x=w/6; x<5*w/6; x++, px+=4)
		{	
			int idx =	((px[0]/col_thres_rg)) +
						((px[1]/col_thres_rg)*3) +
						((px[2]>col_thres_b?1:0)*9);

			hist[idx].r += px[0];					
			hist[idx].g += px[1];
			hist[idx].b += px[2];
			hist[idx].count ++;

			tot.r += px[0];					
			tot.g += px[1];
			tot.b += px[2];
			tot.count ++;
		}
	}

	qsort(hist, sizeofArray(hist), sizeof(color_stat), color_stat::comp);

	json = "[\n";
	int taken = 0;
	for(int i=0; i<(int)sizeofArray(hist) && taken < (int)(tot.count*2/3); i++)
	{
		if(taken)json += ",\n";
		char text[200];

		int raw[3];
		raw[0] = hist[i].r/hist[i].count;
		raw[1] = hist[i].g/hist[i].count;
		raw[2] = hist[i].b/hist[i].count;

		int label = 0;
		sprintf(text, "{\"rgb\": [ %d,%d,%d ], \"label\": %d, \"name\": \"%s\", \"ratio\": %0.3f}",
						raw[0], raw[1], raw[2], label,
						rgb2label(raw, &label),
						hist[i].count/(float)tot.count
		);
	
		json += text;
		taken += hist[i].count;
	}

	json += "\n]";
}

void rt::UnitTests::ipp_major_color()
{
	ipp::Image_4c8u img;
	if(img.Load("c3.jpg"))
	{
		std::string json;

		major_color(img.GetBits(), img.GetWidth(), img.GetHeight(), img.GetStep(), json);
		_LOG(json);
	}
}

void rt::UnitTests::ipp_image()
{
	{	// hook up with OpenCV by referrring the internal pixel buffer from a cv::Mat without memory allocation
		//cv::Mat frame;
		//::ipp::ImageRef_3c8u base(frame.data, frame.cols, frame.rows);
	}

	{	ipp::Image_3c8u img;
		img.Load("test_16.gif");
		img.Save("from_gif_16.png");
		img.Load("test_16_interfaced.gif");
		img.Save("from_test_16_interfaced.png");
		img.Load("test.gif");
		img.Save("from_gif.png");

		ipp::GetEnv()->Push();
		ipp::GetEnv()->GifEncodeColorCount = 64;
		img.Save("from_gif_64.gif");

		ipp::GetEnv()->GifEncodeColorCount = 16;
		img.Save("from_gif_16.gif");
		ipp::GetEnv()->Pop();
	}

	{	ipp::Image_3c8u	img;
		img.SetSize(200,200);

		os::FileRead<BYTE>	data("test_large.jpg");
		ipp::ImageDecoder	file;
		file.Decode(data,(UINT)data.GetSize());  // can be simplified by just using Image::Save/Load

		img.SetSize(file.GetImageWidth(), file.GetImageHeight());
		img = ipp::ImageRef_3c8u(file.GetOutput(), file.GetImageWidth(), file.GetImageHeight(), file.GetImageStep());
		_LOG(img.GetHeight());

		img.Save("test.jpg");
		img.Save("test_large.gif");

		ipp::ImageEncoder	enc;
		enc.Encode((LPCBYTE)img.GetBits(), img.GetChannels(), img.GetWidth(), img.GetHeight(), img.GetStep(), ipp::ImageCodec_PNG);

		os::File("test.png", os::File::Normal_Write).Write(enc.GetOutput(), enc.GetOutputSize());
	}

	{	ipp::ImageDecoder	file;
		os::FileRead<BYTE>	data("test.png");
		file.Decode(data,(UINT)data.GetSize());
		_LOG("PNG Width = "<<file.GetImageWidth());

		ipp::Image_3c8u	img;
		img.SetSize(file.GetImageWidth(), file.GetImageHeight());
		img = ipp::ImageRef_3c8u(file.GetOutput(), file.GetImageWidth(), file.GetImageHeight(), file.GetImageStep());

		ipp::ImageEncoder	enc;
		enc.SetQualityRatio(60);
		enc.Encode((LPCBYTE)img.GetBits(), img.GetChannels(), img.GetWidth(), img.GetHeight(), img.GetStep());

		os::File("dump_png.jpg", os::File::Normal_Write).Write(enc.GetOutput(), enc.GetOutputSize());

		//ipp::Vec3f hist[10];
		//img.HistogramEven(hist, sizeofArray(hist), 255);
		//_LOG(hist[0]);
	}	


	{	ipp::Image_3c8u	img3;
		ipp::Image_1c8u	img1;
		
		img3.Load("test.jpg");
		img3.Save("dump_jpg_1u.gif");
		img3.Save("dump_jpg_3u.png", ipp::ImageCodec_PNG);

		img1.Load("test.jpg");
		img1.Save("dump_jpg_1u.png", ipp::ImageCodec_PNG);

		ipp::GetEnv()->JpegEncodeQuality = 30;
		img3.Load("dump_jpg_1u.png");
		img3.Save("dump_jpg_1u_30.jpg");
	}

	{
		ipp::Image_3c8u	 img;
		img.Load("test.jpg");

		ipp::Image_3c32f hdr(img);

		ipp::Image_1c8u	 img_g(img);
		img_g = hdr;
		img_g.Save("test_g.jpg");

		hdr = img;
        
		hdr.Save("test.pfm");
		hdr.Save("test_piz.exr"); // PIZ is the default
		hdr.Save("test_pxr24.exr", ipp::ImageCodec_EXR_PXR24);
		hdr.Save("test_zlib.exr", ipp::ImageCodec_EXR_ZIP);

		ipp::Image_3c32f	img_hdr;
		img_hdr.Load("test_piz.exr");
		img_hdr.Save("test_sqrt.png");
	}
}

#ifdef PLATFORM_INTEL_IPP_SUPPORT

void rt::UnitTests::ipp_image_apps()
{
	{	// VRF
		float threshold = 0.04f;
		UINT   size_min = 32;

		LPCSTR fn =
			//"bmw_3k.png"
			//"flower1.jpg"
			//"minnie.png"
			"zephyr_2k.png"
			;


		ipp::Image_3c8u	src, work;
		ipp::Image_1c8u sample;
		//src.Load("whirl.jpg");
		//src.Load("minnie_2k.png");
		//src.Load("zephyr.png");
		//src.Load("ae.jpg");
		//src.Load("bmw.png");
		//src.Load("fig.png");
		//src.Load("single_orig.png");

		if(0)
		{	ipp::Image_3c8u sss;
			sss.Load(fn);
			src.SetSize(1462, 1462);
			sss.ResizeTo_SuperSampling(src);
		}
		else
			src.Load(fn);

		rt::Vec3d sum;
		src.Sum(sum);
		sum /= src.GetWidth()*src.GetHeight();
		sum.r = rt::max(sum.r, 10.0);
		sum.g = rt::max(sum.g, 10.0);
		sum.b = rt::max(sum.b, 10.0);

		_LOG(sum);

		sample.SetSize(src);
		sample.Zero();

		rt::Randomizer rng(100);
		float scale = 1;

		while(src.GetWidth()/scale >= size_min && src.GetHeight()/scale >= size_min)
		{
			work.SetSize((UINT)(src.GetWidth()/scale), (UINT)(src.GetHeight()/scale));
			src.ResizeTo_SuperSampling(work);

			int spcount = 0;

			for(UINT y=1;y<work.GetHeight()-1;y++)
			for(UINT x=1;x<work.GetWidth()-1;x++)
			{
				rt::Vec3i	v = work(x,y);
				rt::Vec3i	la;

				/*
				la.r =	rt::Vec4i(
							abs(work(x + 1, y + 0).r + work(x - 1, y + 0).r - 2*v.r),
							abs(work(x - 0, y - 1).r + work(x - 0, y + 1).r - 2*v.r),
							abs(work(x + 1, y + 1).r + work(x - 1, y - 1).r - 2*v.r),
							abs(work(x + 1, y - 1).r + work(x - 1, y + 1).r - 2*v.r)
							).Max()/2;

				la.g =	rt::Vec4i(
							abs(work(x + 1, y + 0).g + work(x - 1, y + 0).g - 2*v.g),
							abs(work(x - 0, y - 1).g + work(x - 0, y + 1).g - 2*v.g),
							abs(work(x + 1, y + 1).g + work(x - 1, y - 1).g - 2*v.g),
							abs(work(x + 1, y - 1).g + work(x - 1, y + 1).g - 2*v.g)
							).Max()/2;

				la.b =	rt::Vec4i(
							abs(work(x + 1, y + 0).b + work(x - 1, y + 0).b - 2*v.b),
							abs(work(x - 0, y - 1).b + work(x - 0, y + 1).b - 2*v.b),
							abs(work(x + 1, y + 1).b + work(x - 1, y - 1).b - 2*v.b),
							abs(work(x + 1, y - 1).b + work(x - 1, y + 1).b - 2*v.b)
							).Max()/2;
				*/


				la.r =	rt::max(
						rt::Vec4i(
							abs(work(x + 1, y + 0).r - v.r),
							abs(work(x - 1, y + 0).r - v.r),
							abs(work(x - 0, y - 1).r - v.r),
							abs(work(x - 0, y + 1).r - v.r)
							).Max()
						,
						rt::Vec4i(
							abs(work(x + 1, y + 1).r - v.r),
							abs(work(x - 1, y - 1).r - v.r),
							abs(work(x + 1, y - 1).r - v.r),
							abs(work(x - 1, y + 1).r - v.r)
							).Max()
						);
					
				la.g = rt::max(
						rt::Vec4i(
							abs(work(x + 1, y + 0).g - v.g),
							abs(work(x - 1, y + 0).g - v.g),
							abs(work(x - 0, y - 1).g - v.g),
							abs(work(x - 0, y + 1).g - v.g)
							).Max()
						,
						rt::Vec4i(
							abs(work(x + 1, y + 1).g - v.g),
							abs(work(x - 1, y - 1).g - v.g),
							abs(work(x + 1, y - 1).g - v.g),
							abs(work(x - 1, y + 1).g - v.g)
							).Max()
						);
					
				la.b = rt::max(
						rt::Vec4i(
							abs(work(x + 1, y + 0).b - v.b),
							abs(work(x - 1, y + 0).b - v.b),
							abs(work(x - 0, y - 1).b - v.b),
							abs(work(x - 0, y + 1).b - v.b)
							).Max()
						,
						rt::Vec4i(
							abs(work(x + 1, y + 1).b - v.b),
							abs(work(x - 1, y - 1).b - v.b),
							abs(work(x + 1, y - 1).b - v.b),
							abs(work(x - 1, y + 1).b - v.b)
							).Max()
						);

				//if(sqrt((double)la.L2NormSqr()) > threshold*sqrt((double)work(x,y).L2NormSqr()))
				//if(abs(la.g) > threshold*rt::max(1,work(x,y).g))
				//if(	rt::max(la.r, rt::max(la.g, la.b)) >
				//	threshold*rt::max(v.r, rt::max(v.g, v.b))
				//)
				if(sqrt((double)la.L2NormSqr()) > threshold*sqrt((double)sum.L2NormSqr()))
				{
					int sx = (int)(x*scale + rng.GetNext()%(int)(scale + 0.5f));
					int sy = (int)(y*scale + rng.GetNext()%(int)(scale + 0.5f));
					sample(sx, sy) = 255;
					spcount++;
				}
			}

			_LOG(work.GetWidth()<<'x'<<work.GetHeight()<<": \\mu = "<<spcount*100.0f/(work.GetWidth()*work.GetHeight()));

			sample.Save(rt::String_Ref(fn) + '_' + scale + ".png", ipp::ImageCodec_PNG);
			scale*=1.4f;
		}

		work.SetSize((UINT)(src.GetWidth()/scale), (UINT)(src.GetHeight()/scale));
		for(UINT y=0;y<work.GetHeight();y++)
		for(UINT x=0;x<work.GetWidth();x++)
		{
			int sx = (int)(x*scale + rng.GetNext()%(int)(scale + 0.5f));
			int sy = (int)(y*scale + rng.GetNext()%(int)(scale + 0.5f));
			sample(sx, sy) = 255;
		}

		sample.Save(rt::String_Ref(fn) + "_final.png", ipp::ImageCodec_PNG);
		rt::Vec<__int64, 1> sumv;
		sample.Sum(sumv);
		_LOG("PX = "<<sumv.x/255);
	}

	return;

	{	// DY's brain training >_<
		int seed = 66165;
		rt::Randomizer rng(seed);

		ipp::Image_3c8u  a,b;
		a.SetSize(100,100);
		rt::Buffer_Ref<BYTE>((LPBYTE)a.GetBits(), a.GetHeight()*a.GetStep()).RandomBits(seed);
		for(UINT i=0;i<a.GetHeight()*a.GetStep();i++)
		{
			((LPBYTE)a.GetBits())[i] = ((BYTE)rng.GetNext())&0xc0;
		}

		b.SetSize(a);
		b = a;
		
		int x = rng.GetNext()%b.GetWidth();
		int y = rng.GetNext()%b.GetHeight();
		b(x,y).r = ~b(x,y).r;
		b(x,y).g = ~b(x,y).g;
		b(x,y).b = ~b(x,y).b;

		ipp::Image_3c8u merged;
		merged.Zero();
		merged.SetSize(a.GetWidth() + b.GetWidth() + 4, b.GetHeight() + 2);
		merged.GetSub(1, 1, a.GetWidth(), a.GetHeight()) = a;
		merged.GetSub(a.GetWidth() + 3, 1, a.GetWidth(), a.GetHeight()) = b;

		ipp::Image_3c8u enlarged;
		enlarged.SetSize(merged.GetWidth()*10, merged.GetHeight()*10);
		merged.ResizeTo_Nearest(enlarged);

		enlarged.Save("see.png", ipp::ImageCodec_PNG);
	}
}


void rt::UnitTests::ipp_imageproc()
{
	{
		ipp::Image_3c8u bg, lab, sh;
		ipp::Image_4c8u h, b;

		bg.Load("b.png");
		h.Load("h.png");

		ipp::Image_4c8u out;
		out.SetSize(h);
		lab.SetSize(h);
		sh.SetSize(h);
		b.SetSize(h);
		ipp::GetEnv()->JpegEncodeQuality = 95;

		srand((UINT)time(0));

		bg.GammaInv();

		for(UINT i=0; i<100; i++)
		{
			{
				float a = 255*rand()/(float)RAND_MAX;
				float b = 255*rand()/(float)RAND_MAX;
				float c = 255*rand()/(float)RAND_MAX;

				for(UINT y=0; y<bg.GetHeight(); y++)
				for(UINT x=0; x<bg.GetWidth(); x++)
				{
					auto& p = bg(x,y);
					auto& d = sh(x,y);

					rt::Vec3f  v = p;
					float bri = v.GetBrightness()*0.9f;
					v.x += a;
					v.y += b;
					//v.z *= c;

					bri /= v.GetBrightness();

					v *= bri;

					d = v;
				}
			}

			sh.Gamma();
			b.CopyFrom(sh);

			out.AlphaBlend(h, b);
			out.Save(rt::SS("hand_") + rt::tos::Number(i).RightAlign(4, '0') + ".png");
		}
	}

	return;

	ipp::Image_3c8u	img, ret, resized;
	img.Load("Konachan.com - 144591 sample.jpg");
	ret.SetSize(img);

	img.GammaTo(ret);		ret.Save("gamma.jpg");
	img.GammaInvTo(ret);	ret.Save("gamma_inv.jpg");
	ret = img;
	ret.BoxFilter(ipp::Size(16,4));	ret.Save("boxfilter.png", ipp::ImageCodec_PNG);

	resized.SetSize(512,512);
	img.ResizeTo_Nearest(resized);
	resized.Save("resized_nearest.jpg");
	img.ResizeTo_Bilinear(resized);
	resized.Save("resized_linear.jpg");
	img.ResizeTo_SuperSampling(resized);
	resized.Save("resized_super.jpg");

	ipp::Image_1c8u r,g,b;
	r.SetSize(resized);
	g.SetSize(resized);
	b.SetSize(resized);
	resized.ChannelSplit(r,g,b);
	r.HaarWaveletFwd(4);
	g.HaarWaveletFwd(4);
	b.HaarWaveletFwd(4);
	resized.ChannelJoin(r,g,b);
	resized.Save("haar_L1.jpg");

	rt::Vec3f	im,bm;
	img.Mean(im);
	_LOG(im);
	img.GaussTo(ret, 8);
	ret.Mean(bm);
	_LOG(bm);
	_LOG(im.x/bm.x);

	ret.Save("gauss_blur.jpg");
}

void rt::UnitTests::ipp_matting()
{
	ipp::Image_3c8u	w,b;

	w.Load("w.png");
	b.Load("b.png");

	if(w.GetRegion() == b.GetRegion())
	{
		ipp::Image_4c8u	out;
		out.SetSize(w);
		for(UINT y=0;y<out.GetHeight();y++)
		for(UINT x=0;x<out.GetWidth();x++)
		{
			rt::Vec4b& pf = out(x,y);
			pf.a = b(x,y).GetBrightness() + 255 - w(x,y).GetBrightness();

			rt::Vec3b& pb = b(x,y);
			if(pf.a)
			{
				pf.r = pb.r*255/pf.a;
				pf.g = pb.g*255/pf.a;
				pf.b = pb.b*255/pf.a;
			}
			else{ pf.r = pf.g = pf.b = 0; }
		}
		out.Save("matt.png",ipp::ImageCodec_PNG);
	}
	else
	{	_LOG("size not match.");
	}
}


#endif // #ifdef PLATFORM_INTEL_IPP_SUPPORT


void rt::UnitTests::ipp_zlib()
{
	os::FileRead<BYTE>	data("test.dat");
	rt::Buffer<BYTE>	comp;
	comp.SetSize(data.GetSize() + 1024);

	UINT outlen = (UINT)comp.GetSize();
	os::TickCount	tc;
	tc.LoadCurrentTick();
	if(rt::zlib_encode(data,(UINT)data.GetSize(), comp, outlen))
		_LOG("Input: "<<rt::tos::FileSize<>(data.GetSize())<<", Output: "<<rt::tos::FileSize<>(outlen));
	_LOG("Time: "<<rt::tos::TimeSpan<>(tc.TimeLapse()));

	rt::Buffer<BYTE>	uncomp;
	uncomp.SetSize(comp.GetSize());

	UINT unziplen = (UINT)uncomp.GetSize();
	if(rt::zlib_decode(comp, outlen, uncomp, unziplen))
	{
		_LOG("Unzip: "<<rt::tos::FileSize<>(unziplen));
		if(unziplen == data.GetSize())
			_LOG("memcmp: "<<memcmp(data, uncomp, unziplen));
	}
}
