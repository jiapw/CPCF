#include "../../core/os/precompiler.h"

#pragma warning(disable: 4838)

#include "test.h"
void rt::UnitTests::precompiler()
{
	rt::String source3 = rt::SS(
		"#if VAL == 1\n"
		"#if true\n"
		"val is 1\n"
		"#endif\n"
		"val is 1 for sure\n"
		"#elif VAL == 2\n"
		"val is 2\n"
		"#elif VAL == 3\n"
		"val is 3\n"
		"#else\n"
		"val > 3\n"
		"#endif\n"
	);

	for (UINT i = 1; i <= 4; i++)
	{
		os::PrecompilerPredefinedMacros predefined;
		predefined.Set("VAL", rt::tos::Number(i));

		os::Precompiler s;
		s.SetEnvVars(&predefined);

		s.Compile("val.cpp", source3);
		//_LOG(source);
		_LOG("\nPrecompiled:");
		_LOG(s.GetCompiled());
	}


	rt::String source = rt::SS(
		"#define TT 234\n"
		"#define IT \"this is the value of IT\"\n"
		"#define IT2 Values\n"
		"#define MultiLine	Line1 \\\n"
		"                   Line2 \\\n"
		"                   Line3 \n"
		"#define Max(a,b)   a>b?a:b\n"
		"#define MERGED     %IT% + %IT2%\n"
		"\n"
		"%Max(1,2)%\n"
		"IT = %IT%%IT2%;\n"
		"MultiLine = %MultiLine%;\n"
		"MultiLine is Line1 \\\n"
		"             Line2 \\\n"
		"             %COMPUTERNAME%\n"
		"\n"
		"ComSpec = %ComSpec%%HOMEDRIVE%%SomeMissing%\n"
		"%NotClosed %HOMEPATH%%%\n"
		"%Max(%COMPUTERNAME%,12)%\n"
		"Merged = %MERGED%\n"
		"#ifdef TT\n"
		"TT = %TT%\n"
		"#ifdef TT2\n"
		"TT2 = %TT2%\n"
		"#endif\n"
		"#endif\n"
		"#ifndef TT\n"
		"TT not found\n"
		"#else\n"
		"TT exists\n"
		"#endif\n"
		"End of File\n\n"
	);

	rt::String source2 = rt::SS(
		"#define TT 234\n"
		"#define IT \"this is the value of it\"\n"
		"#define IT2 Values\n"
		"#define MultiLine	Line1 \\\n"
		"                   Line2 \\\n"
		"                   Line3 \n"
		"#define Max(a,b)   a>b?a:b\n"
		"#define MERGED     IT + IT2 + HAHA##TT##XIXI##Max(2,3)##WIWI\n"
		"\n"
		"Max(1,2)\n"
		"it = IT IT2;\n"
		"multiLine = MultiLine;\n"
		"multiLine is Line1 \\\n"
		"             Line2 \\\n"
		"             COMPUTERNAME\n"
		"\n"
		"comSpec = ComSpec HOMEDRIVE SomeMissing\n"
		"Max(COMPUTERNAME,12)\n"
		"Merged = MERGED\n"
		"#ifdef TT\n"
		"tt = TT\n"
		"#ifdef TT2\n"
		"tt2 = TT2\n"
		"#endif\n"
		"#endif\n"
		"#ifndef TT\n"
		"tt not found\n"
		"#else\n"
		"tt exists\n"
		"#endif\n"
		"#if defined(IT) || defined(TTTT)\n"
		"it is defined\n"
		"#endif\n"
		"#if TT > 100 && TT <= 300\n"
		"tt is fit\n"
		"#endif\n"
		"#if !defined(IT)\n"
		"it is not defined\n"
		"#endif\n"
		"#if IT == \"this is the value of it\"\n"
		"it is correct\n"
		"#endif\n"
		"#if IT != \"this is the value of it\"\n"
		"it is not correct\n"
		"#endif\n"
		"#define TXID(a, b)		(a<<16 | b)\n"
		"TXID(tx.Contract, tx.Op)\n"
		"End of File\n\n"
	);

	os::CommandLine cmd;
	cmd.LoadEnvironmentVariablesAsOptions();

	os::PrecompilerPredefinedMacros predefined;
	predefined.ImportCmdLineOptions(cmd);
	predefined.Set("COMPUTERNAME", "MOOD-PC");
	predefined.Set("ComSpec", "cmd.exe");
	predefined.Set("HOMEDRIVE", "C:");
	predefined.Set("HOMEPATH", "/var/usr/home");

	{	os::Precompiler s("%", "%");
	s.SetEnvVars(&predefined);
	s.Compile("test.bat", source);
	//_LOG(source);
	_LOG("\nPrecompiled:");
	_LOG(s.GetCompiled());
	}

	{	os::Precompiler s;
	s.SetEnvVars(&predefined);
	s.Compile("test.cpp", source2);
	//_LOG(source);
	_LOG("\nPrecompiled:");
	_LOG(s.GetCompiled());
	}

	//os::File("out.cpp", os::File::Normal_WriteText).Write(os::Precompiler("main.cpp").GetCompiled());
}