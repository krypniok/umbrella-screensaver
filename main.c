#include <windows.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <stdio.h>

GLFWwindow* window;

// OpenGL Initialisierung
void initOpenGL() {
    if (!glfwInit()) {
        MessageBox(NULL, "GLFW konnte nicht initialisiert werden!", "Fehler", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    // OpenGL-Kontext und Fenster erstellen
    window = glfwCreateWindow(800, 600, "Bildschirmschoner", NULL, NULL);
    if (!window) {
        glfwTerminate();
        MessageBox(NULL, "Fenster konnte nicht erstellt werden!", "Fehler", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);
}

// Rendering-Funktion
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Beispiel: Ein einfaches rot-drehendes Quadrat
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -6.0f);
    glRotatef((float)glfwGetTime() * 50.0f, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f( 1.0f, -1.0f, 0.0f);
        glVertex3f( 1.0f,  1.0f, 0.0f);
        glVertex3f(-1.0f,  1.0f, 0.0f);
    glEnd();

    glfwSwapBuffers(window);
}

// Aufräumfunktion
void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Die Bildschirmschoner-Prozedur
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            initOpenGL();
            SetTimer(hWnd, 1, 10, NULL);  // Timer alle 10ms
            return 0;

        case WM_DESTROY:
            KillTimer(hWnd, 1);
            cleanup();
            return 0;

        case WM_TIMER:
            render();  // Animation aktualisieren
            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

// Konfigurationsdialog
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                // Hier kannst du Einstellungen speichern
                EndDialog(hDlg, LOWORD(wParam) == IDOK);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Windows-Registrierung (Klassen registrieren)
void RegisterDialogClasses(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = ScreenSaverProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "OpenGLScreenSaver";
    RegisterClass(&wc);
}

// Hauptfunktion für den Bildschirmschoner
int main() {
    // Falls der Bildschirmschoner im Konfigurationsmodus gestartet wird, öffne den Dialog
    if (GetCommandLine()[0] == '-') {
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(1), NULL, ScreenSaverConfigureDialog);
    } else {
        // Registriere die Fensterklasse und starte den Bildschirmschoner
        RegisterDialogClasses(GetModuleHandle(NULL));
        MSG msg;
        HWND hWnd = CreateWindowEx(0, "OpenGLScreenSaver", "OpenGL Screensaver", 0, 0, 0, 800, 600, NULL, NULL, GetModuleHandle(NULL), NULL);
        ShowWindow(hWnd, SW_SHOW);

        // Nachrichtenloop für den Bildschirmschoner
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}
