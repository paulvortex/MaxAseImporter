/*
================================================================================
 
 ASE Importer
 Original code By Christer Janson, Kinetix Development
 Revision by VorteX

================================================================================
*/

#include "Plugin.h"

char replace_basepath[2048];	

#define ASCIIIMP_CLASS_ID1      0x207d2cca
#define ASCIIIMP_CLASS_ID2      0x5df6322c

class AsciiImpClassDesc: public ClassDesc
{
public:
	int IsPublic() { return 1; }
	void *Create(BOOL loading = FALSE) { return new AsciiImp; } 
	const TCHAR *ClassName() { return _T("AsciiImp"); }
	SClass_ID SuperClassID() { return SCENE_IMPORT_CLASS_ID; } 
	Class_ID  ClassID() { return Class_ID(ASCIIIMP_CLASS_ID1, ASCIIIMP_CLASS_ID2); }
	const TCHAR *Category() { return _T("Chrutilities"); }
};

static AsciiImpClassDesc AsciiImpDesc;
ClassDesc* GetAsciiImpDesc() {return &AsciiImpDesc;}

// Class vars
BOOL AsciiImp::resetScene = FALSE;

AsciiImp::AsciiImp()
{
}

AsciiImp::~AsciiImp()
{
}

int AsciiImp::ExtCount()
{
	return 1;
}

const TCHAR * AsciiImp::Ext(int n)
{
	switch(n) {
	case 0:
		return _T("ASE");
	}
	return _T("");
}

const TCHAR * AsciiImp::LongDesc()
{
	return _T("3D Studio MAX ASCII Scene Importer");
}

const TCHAR * AsciiImp::ShortDesc()
{
	return _T("ASCII Scene Import");
}

const TCHAR * AsciiImp::AuthorName() 
{
	return _T("Christer Janson / Pavel [VorteX] Timofeyev");
}

const TCHAR * AsciiImp::CopyrightMessage() 
{
	return _T("Copyight (c) 1997 by Kinetix");
}

const TCHAR * AsciiImp::OtherMessage1() 
{
	return _T("");
}

const TCHAR * AsciiImp::OtherMessage2() 
{
	return _T("");
}

unsigned int AsciiImp::Version()
{
	return 100;
}

static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			CenterWindow(hWnd, GetParent(hWnd));
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hWnd, 1);
					break;
			}
			break;
		default:
			return false;
	}
	return true;
}       

void AsciiImp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}

static INT_PTR CALLBACK ImportDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	AsciiImp *imp = (AsciiImp*)GetWindowLongPtr(hWnd, GWLP_USERDATA); 

	switch(message)
	{
		case WM_INITDIALOG:
			imp = (AsciiImp*)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam); 
			CenterWindow(hWnd, GetParent(hWnd)); 
			SetDlgItemText(hWnd, IDC_BASEPATH, "W:\\SVN\\bo1\\kain");
			CheckDlgButton(hWnd, IDC_RESETSCENE, imp->resetScene); 
			return TRUE;
		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDOK:
						imp->resetScene = IsDlgButtonChecked(hWnd, IDC_RESETSCENE); 
						PostMessage(hWnd, WM_CLOSE, 0, 0);
						break;
					case IDCANCEL:
						EndDialog(hWnd, 0);
						break;
				}
			}
			return TRUE;
		case WM_CLOSE:
			GetDlgItemText(hWnd, IDC_BASEPATH, replace_basepath, sizeof(replace_basepath));
			EndDialog(hWnd, 1);
			return TRUE;
	}
	return FALSE;
}       

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

extern TCHAR* GetToken(FILE* pStream);

// Start the importer!
int AsciiImp::DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts) 
{
	int		lastProgress = 0;
	int		progress;
	BOOL	fileValid = FALSE;
	HWND	importWindow;

	mtlTab.ZeroCount();
	mtlTab.Shrink();

	// Grab the interface pointer.
	ip = i;
	impip = ii;
	
	// Prompt the user with our dialogbox.
	if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ASCIIIMPORT_DLG), GetActiveWindow(), ImportDlgProc, (LPARAM)this))
		return 1;

	// Open the stream
	pStream = fopen(name, "r");
	if (!pStream)
	{
		MessageBox(NULL, "Failed to open file for reading.", "Error", MB_OK);
		return 0;
	}

	// Get the file size
	fseek(pStream, 0, SEEK_END);
	long fsize = ftell(pStream);
	fseek(pStream, 0, SEEK_SET);
	if (resetScene)
		impip->NewScene();

	TCHAR* token = GetToken();
	if (token)
	{
		if (Compare(token, ID_FILEID))
		{
			fileVersion = (int)GetFloat();
			if (fileVersion >= 200 && fileVersion <= 200)
				fileValid = TRUE;
		}
	}
	if (!fileValid)
	{
		MessageBox(NULL, "Not an ASCII version 200 file.", "Error", MB_OK);
		fclose(pStream);
		return 0;
	}

	importWindow = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_IMPORTING), GetActiveWindow(), 0);

#ifndef USE_IMPNODES
	ip->DisableSceneRedraw();
#endif

	long fpos;
	while ((token = GetToken()))
	{
		// Update the progress meter
		// We will eliminate flicker by only calling this when it changes.
		fpos = ftell(pStream);
		progress = 100*fpos/fsize;
		//if (progress != lastProgress)
		//	SetDlgItemInt(importWindow, IDC_IMPORTING_PROGR, progress, true);
		lastProgress = progress;

		if (Compare(token, ID_SCENE))
			ImportSceneParams();
		else if (Compare(token, ID_MATERIAL_LIST))
			ImportMaterialList();
		else if (Compare(token, ID_GEOMETRY))
			ImportGeomObject();
		else if (Compare(token, ID_SHAPE))
			ImportShape();
		else if (Compare(token, ID_HELPER))
			ImportHelper();
		else if (Compare(token, ID_CAMERA))
			ImportCamera();
		else if (Compare(token, ID_LIGHT))
			ImportLight();
		else if (Compare(token, ID_COMMENT))
			GetToken();
#ifndef USE_IMPNODES
		// ImpNodes doesn't support group creation!
		else if (Compare(token, ID_GROUP))
		{
			groupMgr.BeginGroup(GetString());
			GetToken();		// BlockBegin
			GetToken();		// GroupDummy HelperObject
			GetToken();		// GroupDummy BlockBegin
			SkipBlock();	// GroupDummy
		}
		else if (Compare(token, "}")) {
			groupMgr.EndGroup(ip);
		}
#endif
	}

	fclose(pStream);
	
#ifndef USE_IMPNODES
	ip->EnableSceneRedraw();
#endif

	DestroyWindow(importWindow);
	ip->ProgressEnd();
	return 1;
}


