#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <wingdi.h>
#include "resource.h"




#define DRAW_AREA_UPPER_LEFT_X 20
#define DRAW_AREA_UPPER_LEFT_Y 20
#define DRAW_AREA_LOWER_RIGHT_X 570
#define DRAW_AREA_LOWER_RIGHT_Y 520
#define DRAW_AREA_WIDTH 550
#define DRAW_AREA_HEIGHT 500


#define COLOR_TO_BE_TRANSPARENT RGB(153, 255, 204)


#define SRCBUTTONID 13
#define SNKBUTTONID 14
#define MEDBUTTONID 15

typedef struct tagVertex{
	int vNumber;
	RECT vRect;
} VERTEX;



#define VERTEX_MAX 20
VERTEX vertices[VERTEX_MAX];
int v_ind = 0;
int mediate_ind = 0;



short int SRC_IS_SELECTED = 0;
short int SRC_CREATED = 0;

short int SNK_IS_SELECTED = 0;
short int SNK_CREATED = 0;

short int MED_IS_SELECTED = 0;
short int MED_CREATED = 0;

short int DRAG_ACTIVE = 0;

static HBITMAP HSRCBitmap;
static HBITMAP HSRCMonoBitmap;
static HBITMAP HSelectedSRCBitmap;


static HBITMAP HSNKBitmap;
static HBITMAP HSNKMonoBitmap;
static HBITMAP HSelectedSNKBitmap;

VERTEX* firstChosenP  = NULL;
VERTEX* secondChosenP = NULL;

VERTEX* sinkP = NULL;
VERTEX* sourceP = NULL;

void RedrawDrawAreaBackground(HDC hdc) {
	RECT rect;

	SetRect(&rect, DRAW_AREA_UPPER_LEFT_X, DRAW_AREA_UPPER_LEFT_Y, DRAW_AREA_LOWER_RIGHT_X, DRAW_AREA_LOWER_RIGHT_Y);
	FillRect(hdc, &rect, (HBRUSH)(COLOR_3DLIGHT + 1));
}



int CreateVertex(int x, int y) {
	if (v_ind == VERTEX_MAX) {
		return -VERTEX_MAX;
	} else {

		if (SRC_IS_SELECTED) {

			if (SRC_CREATED) {
				return -1;
			}
			else {
				vertices[v_ind].vNumber = v_ind;
				sourceP = &vertices[v_ind];
				SRC_CREATED = 1;
			}

		}

		if (SNK_IS_SELECTED) {

			if (SNK_CREATED) {
				return -2;
			}
			else {
				vertices[v_ind].vNumber = v_ind;
				sinkP = &vertices[v_ind];
				SNK_CREATED = 1;
			}
		}

		if (SRC_IS_SELECTED || SNK_IS_SELECTED || MED_IS_SELECTED) {
			SetRect(&vertices[v_ind].vRect, x - 21, y - 21, x + 21, y + 21);
			v_ind++;
			return 0;
		}
		else {
			return -5;
		}
		
	}
}


void DrawVertex(HDC hdc, VERTEX* v) {
	HDC hMemDC = CreateCompatibleDC(hdc);
	int x = v->vRect.left;
	int y = v->vRect.top;

	if (v == sourceP)
		SelectObject(hMemDC, HSRCBitmap);
	else if (v == sinkP)
		SelectObject(hMemDC, HSNKBitmap);

	TransparentBlt(hdc, x, y, 41, 41,
		hMemDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);

	DeleteDC(hMemDC);
}


void DrawSelectedVertex(HDC hdc, VERTEX* v) {
	HDC hMemDC = CreateCompatibleDC(hdc);
	int x = v->vRect.left;
	int y = v->vRect.top;

	if (v == sourceP)
		SelectObject(hMemDC, HSelectedSRCBitmap);
	else if (v == sinkP)
		SelectObject(hMemDC, HSelectedSNKBitmap);

	TransparentBlt(hdc, x, y, 41, 41,
		hMemDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);

	DeleteDC(hMemDC);
}

void DrawAllVerticesBack(HDC hdc) {
	HDC hMemDC = CreateCompatibleDC(hdc);

	RedrawDrawAreaBackground(hdc);

	for (int i = 0; i < v_ind; i++) {
		int x = vertices[i].vRect.left;
		int y = vertices[i].vRect.top;
		
		if (firstChosenP == &vertices[i]) {
			if (&vertices[i] == sourceP)
				SelectObject(hMemDC, HSelectedSRCBitmap);
			else if (&vertices[i] == sinkP)
				SelectObject(hMemDC, HSelectedSNKBitmap);
		}
		else {

			if (&vertices[i] == sourceP)
				SelectObject(hMemDC, HSRCBitmap);
			else if (&vertices[i] == sinkP)
				SelectObject(hMemDC, HSNKBitmap);
		}

		TransparentBlt(hdc, x, y, 41, 41,
			hMemDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);
	}

	DeleteDC(hMemDC);
}

void DeselectButton(HWND hWnd, short int *btn) {
	HCURSOR hCurs = LoadCursor(NULL, IDC_ARROW);
	SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG)(hCurs));
	*btn = 0;
}

void DeselectButtonsIfAny(HWND hWnd) {
	if (SRC_IS_SELECTED) DeselectButton(hWnd, &SRC_IS_SELECTED);
	if (SNK_IS_SELECTED) DeselectButton(hWnd, &SNK_IS_SELECTED);
	if (MED_IS_SELECTED) DeselectButton(hWnd, &MED_IS_SELECTED);
	InvalidateRect(hWnd, NULL, TRUE);
}

void DeleteVertexFromArray(HDC hdc, VERTEX* p) {

	if (p == sourceP) {
		SRC_CREATED = 0;
		SRC_IS_SELECTED = 0;
		if (sinkP != NULL){
			if (sinkP > sourceP) sinkP--;
		}
		sourceP = NULL;
	}
	else if (p == sinkP) {
		SNK_CREATED = 0;
		SNK_IS_SELECTED = 0;
		if (sourceP != NULL){
			if (sourceP > sinkP) sourceP--;
		}
		sinkP = NULL;
	}

	for (int i = 0; i < v_ind; i++) {
		if (&vertices[i] == p) {
			int ind = i;
			for (int j = ind + 1; j < v_ind; j++) {
				vertices[j - 1] = vertices[j];
			}
			v_ind--;
			break;
		}
	}

	DrawAllVerticesBack(hdc);
}

void ProcessClickInDrawArea(HWND hWnd, HDC hdc, int x, int y) {
	if (SRC_IS_SELECTED || SNK_IS_SELECTED || MED_IS_SELECTED) {

		int res = CreateVertex(x, y);
		if (res == -VERTEX_MAX) {
			MessageBoxA(hWnd, "2 many", "Wrong!", (UINT)NULL);
		}
		else if (res == -1) {
			MessageBoxA(hWnd, "SRC has been created already", "Wrong!", (UINT)NULL);
		}
		else if (res == -2) {
			MessageBoxA(hWnd, "SNK has been created already", "Wrong!", (UINT)NULL);
		}
		else if (res == 0) {
			DrawVertex(hdc, &vertices[v_ind - 1]);
		}

		DeselectButtonsIfAny(hWnd);
	}
	else {

		RECT rect;
		HRGN hRgn;

		for (int i = 0; i < v_ind; i++) {
			SetRect(&rect, vertices[i].vRect.left, vertices[i].vRect.top, vertices[i].vRect.right, vertices[i].vRect.bottom);
			hRgn = CreateRectRgnIndirect(&rect);

			if (PtInRegion(hRgn, x, y)) {

				DRAG_ACTIVE = 1;
				if (firstChosenP == NULL) {
					firstChosenP = &vertices[i];
					DrawSelectedVertex(hdc, firstChosenP);
				} 
					else 

				if (secondChosenP == NULL) {
					secondChosenP = &vertices[i];
						if (secondChosenP == firstChosenP) {
							DrawVertex(hdc, firstChosenP);
							firstChosenP = NULL;
							secondChosenP = NULL;
						}
						else {
							DrawVertex(hdc, firstChosenP);
							firstChosenP = NULL;
							secondChosenP = NULL;
						}
				}
				DeleteObject(hRgn);
				break;

			}
			else {
				SetWindowText(hWnd, L"Didn't hit!");
			}

			DeleteObject(hRgn);
		}

	}
}


void DrawItemSupp(HDC hMemDC, HWND hWnd, LPDRAWITEMSTRUCT pdis, 
				  short int *globSelectedPointer, short int *globCreatedPointer, 
				  HBITMAP positive, HBITMAP negative) {
					  {
						  switch (pdis->itemAction) {
						  case ODA_SELECT:
							  if ((pdis->itemState & ODS_SELECTED) && !(*globCreatedPointer)) {
								  (*globSelectedPointer) = (*globSelectedPointer) ? 0 : 1;
							  }
						  }

						  if ((*globSelectedPointer)) {
							  SelectObject(hMemDC, negative);
						  }
						  else {
							  if (!(*globCreatedPointer)) {
								  SelectObject(hMemDC, positive);
								  DeselectButton(hWnd, globCreatedPointer);
							  }
							  else
								  SelectObject(hMemDC, negative);
						  }
					  }
}

void DrawItemDependingOnParameters(HWND hWnd, HDC hMemDC, LPDRAWITEMSTRUCT pdis, WPARAM wParam) {
	switch (wParam) {
	case SRCBUTTONID:
		DrawItemSupp(hMemDC, hWnd, pdis, &SRC_IS_SELECTED, &SRC_CREATED, HSRCBitmap, HSRCMonoBitmap);
		break;

	case SNKBUTTONID:
		DrawItemSupp(hMemDC, hWnd, pdis, &SNK_IS_SELECTED, &SNK_CREATED, HSNKBitmap, HSNKMonoBitmap);
		break;
	}
}

LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg) {

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_CREATE: {
		HMENU hMenuBar = CreateMenu();
		HMENU hFile = CreateMenu();
		AppendMenu(hMenuBar, MF_POPUP, (UINT)hFile, L"Файл");
		AppendMenu(hFile, MF_STRING, (UINT_PTR) NULL, L"Открыть");
		AppendMenu(hFile, MF_STRING, (UINT_PTR) NULL, L"Сохранить");
		AppendMenu(hFile, MF_STRING, (UINT_PTR) NULL, L"Сохранить как...");
		AppendMenu(hFile, MF_STRING, (UINT_PTR) NULL, L"Выход");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR) NULL, L"Справка");
		SetMenu(hWnd, hMenuBar);


		HSRCBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SRC_BITMAP));
		HSRCMonoBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SRC_MONO_BITMAP));
		HSelectedSRCBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SELECTED_SRC_BITMAP));

		HSNKBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SNK_BITMAP));
		HSNKMonoBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SNK_MONO_BITMAP));
		HSelectedSNKBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SELECTED_SNK_BITMAP));

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	 
	case WM_COMMAND:{

		switch (LOWORD(wParam)) {
		case SRCBUTTONID:

			if (!SRC_CREATED && SRC_IS_SELECTED) {
				HCURSOR hCurs = LoadCursor(NULL, IDC_CROSS);
				SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG)(hCurs));
			}
			else {
				SRC_IS_SELECTED = 0;
			}
			break;

		case SNKBUTTONID:

			if (!SNK_CREATED && SNK_IS_SELECTED) {
				HCURSOR hCurs = LoadCursor(NULL, IDC_CROSS);
				SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG)(hCurs));
			}
			else {
				SNK_IS_SELECTED = 0;
			}
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_LBUTTONDOWN: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		HDC hdc = GetDC(hWnd);
		RECT rect;

		SetRect(&rect, DRAW_AREA_UPPER_LEFT_X + 30, DRAW_AREA_UPPER_LEFT_Y + 30, DRAW_AREA_LOWER_RIGHT_X - 30, DRAW_AREA_LOWER_RIGHT_Y - 30);
		HRGN hRgn = CreateRectRgnIndirect(&rect);
		SelectClipRgn(hdc, hRgn);


		if (PtInRegion(hRgn, x, y)) {
			SetFocus(hWnd);
			ProcessClickInDrawArea(hWnd, hdc, x, y);
		}
		else {
			DeselectButtonsIfAny(hWnd);
		}


		ReleaseDC(hWnd, hdc);
		DeleteObject(hRgn);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_LBUTTONUP: {
		DRAG_ACTIVE = 0;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		
		RedrawDrawAreaBackground(hdc);

		DrawAllVerticesBack(hdc);

		EndPaint(hWnd, &ps);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_MOUSEMOVE: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		HRGN hRgn;
		RECT drawAreaRect;
		SetRect(&drawAreaRect, DRAW_AREA_UPPER_LEFT_X + 30, DRAW_AREA_UPPER_LEFT_Y + 30, DRAW_AREA_LOWER_RIGHT_X - 30, DRAW_AREA_LOWER_RIGHT_Y - 30);
		hRgn = CreateRectRgnIndirect(&drawAreaRect);

		if (PtInRegion(hRgn, x, y)) {
			if (DRAG_ACTIVE && wParam == MK_LBUTTON && firstChosenP != NULL) {
				SetRect(&(firstChosenP->vRect), x - 21, y - 21, x + 21, y + 21);

				HDC hdc = GetDC(hWnd);
				DrawAllVerticesBack(hdc);
				ReleaseDC(hWnd, hdc);
			}
		}

		DeleteObject(hRgn);

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	case WM_DRAWITEM: {
		LPDRAWITEMSTRUCT pdis;
		pdis = (LPDRAWITEMSTRUCT) lParam;

		HDC hMemDC = CreateCompatibleDC(pdis->hDC);

		DrawItemDependingOnParameters(hWnd, hMemDC, pdis, wParam);

		FillRect(pdis->hDC, &pdis->rcItem, (HBRUSH)(COLOR_WINDOW + 1));
		TransparentBlt(pdis->hDC, 0, 0, 41, 41,
			hMemDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);

		

		DeleteDC(hMemDC);

		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define BACKSPACE_VIRTUAL_CODE 0x08
	case WM_KEYDOWN: {

		if (wParam == BACKSPACE_VIRTUAL_CODE) {
			if (firstChosenP != NULL) {
				HDC hdc = GetDC(hWnd);
				DeleteVertexFromArray(hdc, firstChosenP);
				firstChosenP = NULL;
				DeselectButtonsIfAny(hWnd);
				ReleaseDC(hWnd, hdc);
			}
		}

		return 0;
	}


	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}


	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	WNDCLASSEX wcex;	//Оконный класс для окна нашего приложения

	HWND hWnd;			//Главное окно нашего приложения
	MSG msg;			//Структура для получения и отправки сообщений

	//////////////////////////////////////////////
	//////////////////////////////////////////////			//Заполнение полей оконного класса
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = MyWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"MyWindowClass";
	wcex.hIconSm = 0;
	//////////////////////////////////////////////
	//////////////////////////////////////////////

	RegisterClassEx(&wcex);		//Регистрация оконного класса

	//Создание главного окна приложения
	hWnd = CreateWindowEx(0, L"MyWindowClass", L"Программное средство для нахождения максимального потока в сети",
						  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
						  0, 0, 800, 600, 0, 0, hInstance, NULL);

	//Создание рамки для области рисования
	HWND groupButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", L"Безымянный.nw", 
									  WS_VISIBLE | WS_CHILD | WS_BORDER | BS_GROUPBOX, 
									  DRAW_AREA_UPPER_LEFT_X - 15,
									  DRAW_AREA_UPPER_LEFT_Y - 15,
									  DRAW_AREA_WIDTH + 25,
									  DRAW_AREA_HEIGHT + 25, hWnd, NULL, NULL, NULL);

	//Создание кнопки с источником сети
	HWND srcButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", L"Btn1",
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		640,
		50,
		42,
		42, hWnd, (HMENU) SRCBUTTONID, NULL, NULL);

	//Создание кнопки со стоком сети
	HWND snkButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", L"Btn2",
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		640,
		200,
		42,
		42, hWnd, (HMENU)SNKBUTTONID, NULL, NULL);
	
	//Показ главного окна приложения
	ShowWindow(hWnd, nCmdShow);

	//Цикл обработки сообщений
	while (GetMessage(&msg, 0, 0, 0))
	{
		DispatchMessage(&msg);
	}

	//Возвращение результата работы
	return msg.wParam;
}