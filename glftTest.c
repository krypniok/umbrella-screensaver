#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>

#include <GLFW/glfw3.h>

#include "glftRenderString.h"

#define GLIDBASE glLoadIdentity(); \
glTranslated(0.375, 0.375, 0);

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


#define M_PHI   1.6180339887498948482045868343656381177305180861640625  // Golden Ratio (64-bit double)
#define M_PI2   6.2831853071795864769252867665590057683943387987502116

//#define PHI_KLEINER_100 38.19660112501051517954131656343656359008
//#define PHI_GROESSER_100 61.8033988749894848204586834365635900793

// Alternativ:
#define PHI_100 1.618033988749894848204586834365635900793
#define PHI_KLEINER_100 (PHI_100 - 1)
#define PHI_GROESSER_100 (1 / PHI_100)

#define M_PERFECT 37

char* longstring[] = {
    "long string is too short",
    "long string is too short",
    "long string is too short",
    "long string is too short",
    "long string is too short"
};

typedef struct {
    float x, y, width, height;
} gluiWindow;

typedef struct {
    char text[256];
    int cursor_pos;
    bool active;
} gluiInput;

gluiInput inputField;  // Deklariere eine globale Eingabestruktur

int current_funktion_index = 0;  // Index of the currently running function

// Datentyp für Funktionzeiger
typedef void (*Funktion)(void);

// Struktur für Funktion und Laufdauer
typedef struct {
    Funktion funktion;
    int laufzeit;
} FunktionsEintrag;

void drawYinYang(void);
void funktion5(void);
void funktion1(void);
void funktion2(void);
void funktion3(void);
void funktion4(void);
void gurumeditation(void); 
void no_signal(void); 

// Array von FunktionsEinträgen
FunktionsEintrag funktionen[] = {
    {drawYinYang, 5},
    {funktion5, 300},
    {funktion1, 3},
    {funktion2, 30},
    {funktion3, 8},
    {gurumeditation, 3},
    {no_signal, 3},
    {funktion4, 3}
};

// Anzahl der Funktionen im Array
#define ANZAHL_FUNKTIONEN (sizeof(funktionen) / sizeof(funktionen[0]))



void gluiRenderQuad(float x, float y, float w, float h, int polygonMode);

void gluiDrawScissorBoxOutline(void) {
  // Aktuelle Scissorbox-Werte abrufen
  GLint scissorBox[4];
  glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
  // Linienfarbe festlegen
  glColor3f(1.0f, 1.0f, 0.0f); // Gelb
  gluiRenderQuad((float)scissorBox[0], (float)scissorBox[1], (float)scissorBox[2]-1, (float)scissorBox[3]-16, GL_LINE);
}

void guiRenderInputWindow(gluiWindow window) {
    // Zeichne den Hintergrund des Eingabefelds

    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(window.x, window.y);
    glVertex2f(window.x, window.y + window.height);
    glVertex2f(window.x + window.width, window.y + window.height);
    glVertex2f(window.x + window.width, window.y);
    glEnd();

    // Rendere den Text im Eingabefeld
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(window.x + 5.0f, window.y + window.height - 5.0f);

    glEnable(GL_SCISSOR_TEST); // Enable scissor test
    glScissor(window.x, window.y, window.width, window.width);

    gluiDrawScissorBoxOutline();

    int width = glftRenderText(inputField.text, window.x-32, window.y, 1, 255, 255, 255);

    // Falls das Eingabefeld aktiv ist, zeichne den Cursor
    if (1) { // nputField.active) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float cursor_x = window.x + 5.0f + width;
        glBegin(GL_LINES);
        glVertex2f(cursor_x, window.y + 5.0f);
        glVertex2f(cursor_x, window.y + window.height - 5.0f);
        glEnd();
    }
    glDisable(GL_SCISSOR_TEST); // Disable scissor test after drawing
}

glftFont* dejavu;
glftFont* simpsonsfont;
glftFont* arial;
glftFont* matrix;

int gpause = 0;
int currentfont = 0;
int maxfont = 0;

// Funktion zum Einlesen von Dateinamen in ein Array
int read_filenames(char *dirpath, char ***filenames) {

  // Verzeichnis öffnen
  DIR *dir = opendir(dirpath);
  if (dir == NULL) {
    perror("Verzeichnis konnte nicht geöffnet werden");
    return -1;
  }

  // Anzahl der Dateien ermitteln
  int num_files = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    // Dateiname auf .TTF oder .ttf prüfen
    int len = strlen(entry->d_name);
    if (len >= 4 && (strcmp(entry->d_name + len - 4, ".TTF") == 0 || strcmp(entry->d_name + len - 4, ".ttf") == 0)) {
      num_files++;
    }
  }

  // Speicher für Array allokieren
  *filenames = malloc(sizeof(char *) * num_files);
  if (*filenames == NULL) {
    perror("Speicher konnte nicht alloziert werden");
    closedir(dir);
    return -1;
  }

  // Dateinamen in Array einlesen
  int i = 0;
  rewinddir(dir);
  while ((entry = readdir(dir)) != NULL) {
    // Dateiname auf .TTF oder .ttf prüfen
    int len = strlen(entry->d_name);
    if (len >= 4 && (strcmp(entry->d_name + len - 4, ".TTF") == 0 || strcmp(entry->d_name + len - 4, ".ttf") == 0)) {
      // Dateinamen kopieren
      (*filenames)[i] = malloc(strlen(entry->d_name) + 1);
      strcpy((*filenames)[i], entry->d_name);
      i++;
    }
  }

  // Verzeichnis schließen
  closedir(dir);

  return num_files;
}

// Beispielhafte Verwendung
int setfont() {
  char *dirpath = "./fonts/";
  char **filenames;

  int num_files = read_filenames(dirpath, &filenames);
  if (num_files < 0) {
    return 1;
  }
    maxfont = num_files;

  // Ausgabe der Dateinamen
  for (int i = 0; i < num_files; i++) {
  //  printf("%s\n", filenames[i]);
  }

    //dejavu = glftLoadFont("./fonts/SF Scribbled Sans.ttf", 96);

char filename[255];
strcpy(filename, "./fonts/");
strcat(filename, filenames[currentfont]);
dejavu = glftLoadFont(filename, 96);

    glftSetCurrentFont(dejavu);
// Speicher freigeben
  for (int i = 0; i < num_files; i++) {
    free(filenames[i]);
  }
  free(filenames);

  return 0;
}

// Beispiel für Tastaturereignisse (vereinfacht)
void handleKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ENTER) {
            // Hier könntest du Code hinzufügen, um mit dem eingegebenen Text zu arbeiten
            printf("Eingegebener Text: %s\n", inputField.text);
            gpause = 1;
            usleep(500 * 1000);
//            strcpy(string_to_render, inputField.text);
            gpause = 0;
            // Zurücksetzen des Eingabefelds
            memset(inputField.text, 0, sizeof(inputField.text));
            inputField.cursor_pos = 0;
        }
        else if (key == GLFW_KEY_SPACE) {
  current_funktion_index = (current_funktion_index + 1) % ANZAHL_FUNKTIONEN;
//  start_time_functions[current_funktion_index] = glfwGetTime();  // Reset start time for the new function

        }
        else if (key == GLFW_KEY_BACKSPACE) {
            // Lösche das letzte Zeichen
            if (inputField.cursor_pos > 0) {
                inputField.text[--inputField.cursor_pos] = '\0';
            }
        }
        else if (key == GLFW_KEY_PAGE_UP) {
            if(currentfont < maxfont-1) currentfont++;
            setfont();
        }
        else if (key == GLFW_KEY_PAGE_DOWN) {
            if(currentfont > 0) currentfont--;
            setfont();
        }
    }
}

// Beispiel für Zeichenerfassung (vereinfacht)
void handleCharInput(GLFWwindow* window, unsigned int codepoint) {
    if (inputField.cursor_pos < sizeof(inputField.text) - 1) {
        inputField.text[inputField.cursor_pos++] = (char)codepoint;
        inputField.text[inputField.cursor_pos] = '\0';
    }
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void gluiRenderQuad(float x, float y, float w, float h, int polygonMode) {
    // Setze den Zeichenmodus
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    GLIDBASE;

    //glColor3f(0, 1, 0);
    // Beginne ein neues Quad
    glBegin(GL_QUADS);
        // Setze die Eckpunkte des Quads
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        // Beende das Quad
    glEnd();
    
    // Setze den Zeichenmodus zurück
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLoadIdentity();
}

typedef struct {
  float r;
  float g;
  float b;
} RGB;

RGB rainbow_color(float vector) {
  // Define six base colors for the rainbow spectrum
  static const RGB base_colors[] = {
      {1.0f, 0.0f, 0.0f}, // Red
      {1.0f, 0.5f, 0.0f}, // Orange
      {1.0f, 1.0f, 0.0f}, // Yellow
      {0.0f, 1.0f, 0.0f}, // Green
      {0.0f, 0.0f, 1.0f}, // Blue
      {0.5f, 0.0f, 1.0f}, // Indigo
      {1.0f, 0.0f, 1.0f}, // Violet
  };

  // Calculate the number of segments (one less than the number of colors)
  int segments = sizeof(base_colors) / sizeof(base_colors[0]) - 1;

  // Clamp the vector between 0.0 and 1.0
  vector = fmodf(vector, 1.0f);  // Restrict to 0.0-1.0 range using modulo

  // Calculate the segment where the vector falls
  int segment = (int)(vector * segments);

  // Offset within the current segment
  float offset = vector * segments - segment;

  // Blend between the two base colors of the current segment
  RGB color1 = base_colors[segment];
  RGB color2 = base_colors[segment + 1];
  RGB color3 = {
      color1.r * (1.0f - offset) + color2.r * offset,
      color1.g * (1.0f - offset) + color2.g * offset,
      color1.b * (1.0f - offset) + color2.b * offset,
  };
  // printf("r = %f, g = %f, b= %f\n", color3.r, color3.g, color3.b);
  return color3;
}

// Function to calculate a smooth transition between two colors (lerp)
float lerp(float a, float b, float t) {
  return (1.0f - t) * a + t * b;
}

float rotation = 0.0f;
float frequency = 1.66;//1.618f*1;  // Initial frequency
float amplitude = 40.0f-16;  // Initial amplitude
float char_x_pos = WINDOW_WIDTH; // Position des Zeichens auf der X-Achse

int win_width = WINDOW_WIDTH;

void gluiRenderCharSinus(char c, float x) {
        // Berechne die Y-Position des Zeichens basierend auf der Sinuskurve
        float char_y_pos = (WINDOW_HEIGHT/2) + amplitude * sin(frequency * M_PI_2 * x / win_width);
        // Berechne die Steigung der Tangente an der aktuellen Position
        float slope = cos(frequency * 2.0f * M_PI * x / win_width);
        float angle = 180.0f / M_PI * atan2(slope, 1.0f);
        glftRenderCharRotated(c, x, char_y_pos+16, 1, angle);
}







void gluiRenderRoundedRect(float x, float y, float width, float height, float radius) {
}

// Function to draw a filled circle using triangle fan
void DrawFilledCircle2(GLint xCenter, GLint yCenter, GLfloat radius) {
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(xCenter, yCenter);  // Center point for filled circle

    for (GLfloat angle = 0.0f; angle <= M_PI2; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
// Add the last vertex explicitly at angle 2.0f * PI
glVertex2f(xCenter + radius * cos(M_PI2), yCenter + radius * sin(M_PI2));

    glEnd();
}

// Function to draw a filled circle using triangle fan
void DrawFilledCircle(GLint xCenter, GLint yCenter, GLfloat radius) {
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(xCenter, yCenter);  // Center point for filled circle

    for (GLfloat angle = 0.0f; angle < M_PI2; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
// Add the last vertex explicitly at angle 2.0f * PI
glVertex2f(xCenter + radius * cos(M_PI2), yCenter + radius * sin(M_PI2));

    glEnd();
}

void DrawAlternatingCircleSegments(GLint xCenter, GLint yCenter, GLfloat radius, GLfloat segmentAngle) {
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(xCenter, yCenter); // Center point

    // Loop for drawing 8 segments (adjust segmentAngle for desired arc size)
    GLfloat currentAngle = 0.0f;
    for (int i = 0; i < 8; i++) {
        GLfloat segmentColor = (i % 2 == 0) ? 1.0f : 0.0f; // White for even segments, red for odd

        // Calculate segment endpoints
        GLfloat x1 = xCenter + radius * cos(currentAngle);
        GLfloat y1 = yCenter + radius * sin(currentAngle);
        GLfloat x2 = xCenter + radius * cos(currentAngle + segmentAngle);
        GLfloat y2 = yCenter + radius * sin(currentAngle + segmentAngle);

        // Set vertex color based on segment index
        glColor3f(segmentColor, segmentColor, segmentColor);

        // Add vertices for segment
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);

        currentAngle += segmentAngle;
    }

    glEnd();
}


// Function to draw a circle outline using line loop
void DrawCircle(GLint xCenter, GLint yCenter, GLfloat radius) {
    glBegin(GL_LINE_LOOP);

    for (GLfloat angle = 0.0f; angle <= 2.0f * M_PI; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }

    glEnd();
}



double time_offset = 0.0;
void funktion1(void) {
    // Draw the sine curve
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the sine curve
    glBegin(GL_LINE_STRIP);
    for (int x = 0; x <= win_width; x++) {
        float y = amplitude * sin(frequency * 2.0f * M_PI * (float)x / win_width - time_offset);
        glVertex2f(x, (WINDOW_HEIGHT/2) + y);
    }
    glEnd();
    time_offset += 0.1f;
}

void DrawAlternatingCurvedSegments(GLint xCenter, GLint yCenter, GLfloat radius, GLfloat segmentAngle, GLfloat bulgeFactor) {
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(xCenter, yCenter); // Center point

    // Loop for drawing 8 segments
    GLfloat currentAngle = 0.0f;
    for (int i = 0; i < 8; i++) {
        // Calculate segment color (white for even, red for odd)
        (i % 2 == 0) ? glColor3f(1, 1, 1) : glColor3f(1, 0, 0);


        // Calculate segment endpoints
        GLfloat x1 = xCenter + radius * cos(currentAngle);
        GLfloat y1 = yCenter + radius * sin(currentAngle);
        GLfloat x2 = xCenter + radius * cos(currentAngle + segmentAngle);
        GLfloat y2 = yCenter + radius * sin(currentAngle + segmentAngle);

        // Calculate bulge control points
        GLfloat bulgeOffset = bulgeFactor * radius * sin(segmentAngle / 2.0f);
        GLfloat bulgeX1 = xCenter + (radius + bulgeOffset) * cos(currentAngle);
        GLfloat bulgeY1 = yCenter + (radius + bulgeOffset) * sin(currentAngle);
        GLfloat bulgeX2 = xCenter + (radius + bulgeOffset) * cos(currentAngle + segmentAngle);
        GLfloat bulgeY2 = yCenter + (radius + bulgeOffset) * sin(currentAngle + segmentAngle);


        // Add vertices for the segment
        glVertex2f(x1, y1);
        glVertex2f(bulgeX1, bulgeY1);
        glVertex2f(x2, y2);
        glVertex2f(bulgeX2, bulgeY2);

        currentAngle += segmentAngle;
    }

    glEnd();
}

void funktion2(void) {
  //  gluiRenderRoundedRect(100, 100, 100, 100, 25);
  //  DrawCircle(100, 100, 64);
  //  DrawFilledCircle(200, 200, 64);
    DrawAlternatingCurvedSegments(300, 300, 128, M_PI/4, 0);
}

void funktion3(void) {
    glftSetCurrentFont(simpsonsfont);
    int len = strlen(longstring) * dejavu->size;
    // Bewege das Zeichen von rechts nach links entlang der Sinuskurve
    // char_x_pos -= 1.5f; // Verringere die X-Position des Zeichens
    if (char_x_pos < -(len)) { // Wenn die linke Kante erreicht ist
        char_x_pos = win_width; // Setze die X-Position auf die rechte Kante
    }
    float movement_speed = M_PHI; // Adjust this base speed as needed
    // float min_speed = 1.0f; // Minimum speed to prevent complete stop
    // float slope = cos(frequency * 2.0f * M_PI * char_x_pos / win_width);
    char_x_pos -= (movement_speed ); //* fabs(slope)) + min_speed;
    float char_spacing = dejavu->size;  // Adjust
    amplitude=37.5;
    // float time_offset = glfwGetTime();  // Get elapsed time in seconds (optional for animation)
    float position_offset = 0;
    float color_step = 1.0f / 7.0f; // Assuming 7 colors in the rainbow
    for (int i = 0; i < strlen(longstring); i++) {
        float color_offset = i * color_step;
        float r = sin(color_offset) * 0.5f + 0.5f;  // Example color calculations (adjust as needed)
        float g = cos(color_offset) * 0.5f + 0.5f;
        float b = sin(color_offset + 2.0f * M_PI / 3.0f) * 0.5f + 0.5f;
        glColor3f(r, g, b);
        
        //fmodf(time_offset * movement_speed + (float)i / (float)strlen(longstring), 1.0f);
        position_offset += 0.031415;
        RGB color = rainbow_color(position_offset);  // Calculate color based on position
        glColor3f(color.r, color.g, color.b);
        // glTranslated(time_offset, 0, 0);
        gluiRenderCharSinus(toupper(longstring[i]), char_x_pos+(char_spacing*i));
        // glLoadIdentity();
    }

}


char* gurumediation[] = {"Software Failure.  Press left mouse button to continue.", "Guru Meditation #DEADBEEF.38317135"};
void gurumeditation(void) {
    glftSetCurrentFont(dejavu);
    int width = glftMeasureString(gurumediation[0], 1);
    glftRenderText(gurumediation[0], (WINDOW_WIDTH-width)/2, (WINDOW_HEIGHT-96)/2, 1, 192, 32, 8);
    width = glftMeasureString(gurumediation[1], 1);
    glftRenderText(gurumediation[1], (WINDOW_WIDTH-width)/2, ((WINDOW_HEIGHT-96)/2)+dejavu->size, 1, 192, 32, 8);
 
    glLineWidth(8.0);
    glColor3f(0.66, 0, 0);
    gluiRenderQuad(400, 400, 1920-800, 1080-800, GL_LINE);
    glLineWidth(1.0);

}

char nosignal[] = "No Signal";
void no_signal(void) {
    int width = glftMeasureString(nosignal, 1);
    glColor3f(0.15, 0.3, 0.6);
    gluiRenderQuad((1920-350)/2, (1080-150)/2, 350, 150, GL_FILL);

    glftSetCurrentFont(arial);
    glftRenderText(nosignal, (WINDOW_WIDTH-width)/2, (WINDOW_HEIGHT-16)/2, 1, 255, 255, 255);
}

void funktion4(void) {
    const char *asciiString = " !\"#$%&'()*+,-./0123456789:;<=>?@"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                              "abcdefghijklmnopqrstuvwxyz{|}~\x7F";

        glftRenderText(asciiString, 0.0f, 0.0f, 1.0f, 255, 255, 255);
        glftRenderText(dejavu->font_name, 0.0f, 16.0f, 1.0f, 0, 255, 0);

        glColor3f(0, 1, 1);
        glftRenderCharRotated('A', 400, 300-6, 1, rotation++);

        glColor3f(1, 1, 1);
        gluiRenderQuad(100.0f, 100.0f, 600.0f, 400.0f, GL_LINE);

        // Aufruf deiner GUI-Funktion
        guiRenderInputWindow((gluiWindow){200.0f, 200.0f, 200.0f, 30.0f});

        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2f(0, 300);
        glVertex2f(800, 300);
        glEnd();
}


// Function to draw a circle outline using line loop
void DrawCircleS(GLint xCenter, GLint yCenter, GLfloat radius) {
    glBegin(GL_POLYGON);
    glVertex2f(1920/2, 1080/2);
    for (GLfloat angle = 0.0f; angle <=  M_PI/4; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }

    glEnd();
}

void DrawCircleS2(GLint xCenter, GLint yCenter, GLfloat radius, GLfloat angle_start, GLfloat angle_end) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

//    glLoadIdentity();
//    glTranslated(1929/2-4, 1080/2, 0);
//    glRotated(22.5f, 0, 0, 1);

    glBegin(GL_POLYGON);
    
//    glVertex2f(xCenter, yCenter);
      glVertex2f((1920/2), 1080/2);

    for (GLfloat angle = angle_start; angle <= angle_start; angle+=0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
    for (GLfloat angle = angle_start+0.1f; angle < angle_end; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
    for (GLfloat angle = angle_end; angle < angle_end+0.1f; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }

    glEnd();
}

float degreesToRadiansAndWrap(float angleInDegrees) {
  float adjustedAngle = fmodf(angleInDegrees, 360.0f); // Wrap angle between 0 and 360
  if (adjustedAngle < 0.0f) {
      adjustedAngle += 360.0f; // Handle negative values
  }
  return adjustedAngle * (M_PI / 180.0f); // Convert to radians
}


void DrawCircleS3x(GLint xCenter, GLint yCenter, GLfloat radius, GLfloat angle_start, GLfloat angle_end) {
    //glVertex2f(xCenter, yCenter);
    for (GLfloat angle = angle_start; angle <= angle_start; angle+=0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
    for (GLfloat angle = angle_start+0.1f; angle < angle_end; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
    for (GLfloat angle = angle_end; angle < angle_end+0.1f; angle += 0.1f) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
}

void DrawCircleS3(GLint xCenter, GLint yCenter, GLfloat radius, GLfloat angle_start, GLfloat angle_end) {
    for (GLfloat angle = angle_start; angle <= angle_end; angle += 0.1f) { // Hier korrigieren: <= statt < und Inkrementierung innerhalb der Schleife
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
}

void DrawCircleCCW(GLint xCenter, GLint yCenter, GLfloat radius, GLfloat angle_start, GLfloat angle_end) {
    for (GLfloat angle = angle_end; angle >= angle_start; angle -= 0.1f) { // Hier korrigieren: >= statt > und Inkrementierung innerhalb der Schleife
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        glVertex2f(x, y);
    }
}


void DrawEightCircleSegments3(GLfloat x, GLfloat y, GLfloat radius, int segment) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        DrawCircleS3(x, y, radius, degreesToRadiansAndWrap(90), degreesToRadiansAndWrap(270));
        DrawCircleS3(x, y-(radius/2), radius/2, degreesToRadiansAndWrap(270), degreesToRadiansAndWrap(359));
        DrawCircleS3(x, y-(radius/2), radius/2, degreesToRadiansAndWrap(0), degreesToRadiansAndWrap(90));

          DrawCircleCCW(x, y+(radius/2), radius/2, degreesToRadiansAndWrap(90), degreesToRadiansAndWrap(269));
//        glVertex2f(x, y+(radius));
//         glVertex2f(1920/2, 1080/2);
    glEnd();
}

void DrawYinYang(GLfloat x, GLfloat y, GLfloat radius) {
    // Draw the large circle (counter-clockwise)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Set fill mode for large circle
    glBegin(GL_POLYGON);
    for (GLfloat angle = 180.0f; angle >= 0.0f; angle -= 0.1f) {
        GLfloat x_large = x + radius * cos(degreesToRadiansAndWrap(angle));
        GLfloat y_large = y + radius * sin(degreesToRadiansAndWrap(angle));
        glVertex2f(x_large, y_large);
    }
    glEnd();

    // Draw the small black circle (counter-clockwise)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Set fill mode for black circle
    glColor3f(0.0f, 0.0f, 0.0f); // Set color to black
    glBegin(GL_POLYGON);
    for (GLfloat angle = 0.0f; angle <= 180.0f; angle += 0.1f) {
        GLfloat x_small_black = x + (radius / 2) * cos(degreesToRadiansAndWrap(angle));
        GLfloat y_small_black = y + (radius / 2) * sin(degreesToRadiansAndWrap(angle));
        glVertex2f(x_small_black, y_small_black);
    }
    glEnd();

    // Draw the small white circle (clockwise)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Set fill mode for white circle
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    glBegin(GL_POLYGON);
    for (GLfloat angle = 360.0f; angle >= 180.0f; angle -= 0.1f) {
        GLfloat x_small_white = x - (radius / 3) * cos(degreesToRadiansAndWrap(angle));
        GLfloat y_small_white = y + (radius / 3) * sin(degreesToRadiansAndWrap(angle));
        glVertex2f(x_small_white, y_small_white);
    }
    glEnd();
}


void DrawEightCircleSegments(GLint windowWidth, GLint windowHeight, GLfloat radius) {
  // Set initial color to white
  glColor3f(1.0f, 1.0f, 1.0f);

  // Calculate angle increment for 8 equal segments
  GLfloat angleIncrement = 2.0f * M_PI / 8.0f;

  // Calculate center point of the window
  GLint xCenter = windowWidth / 2;
  GLint yCenter = windowHeight / 2;

  // Flag for alternating colors (start with white)
  bool isWhite = true;

  // Draw all 8 segments using a loop
  for (int i = 0; i < 8; i++) {
    GLfloat angle_start = i * angleIncrement;
    GLfloat angle_end = angle_start + angleIncrement;

    // Set color based on alternating flag
    if(isWhite) {
        glColor3f(1, 1, 1);
    }
    else {
//        glColor3f((1/100)*50, 0, 0);
    }

if(i == (0)) {
    // Draw the current circle segment
    DrawCircleS2(xCenter, yCenter, radius, angle_start, angle_end);
}
    // Toggle color flag for next segment
    isWhite = !isWhite;
  }
}

void DrawEightCircleSegments2(GLfloat x, GLfloat y, GLfloat radius, int segment) {
  // Set initial color to white
  glColor3f(1.0f, 1.0f, 1.0f);

  // Calculate angle increment for 8 equal segments
  GLfloat angleIncrement = 2.0f * M_PI / 8.0f;

  // Calculate center point of the window
  //GLint xCenter = 1920 / 2;
  //GLint yCenter = 1080 / 2;

  // Flag for alternating colors (start with white)
  bool isWhite = true;

  // Draw all 8 segments using a loop
  for (int i = 0; i < 8; i++) {
    GLfloat angle_start = i * angleIncrement;
    GLfloat angle_end = angle_start + angleIncrement;

    // Set color based on alternating flag
    if(isWhite) {
        glColor3f(1, 1, 1);
    }
    else {
        glColor3f(1, 0, 0);
    }

    // Draw the current circle segment
    if(i == (segment)) {
    DrawCircleS2(x, y, radius, angle_start, angle_end);
    }

    // Toggle color flag for next segment
    isWhite = !isWhite;
  }
}

// Function to draw a circle outline using line loop
void DrawCircle3(GLint xCenter, GLint yCenter, GLfloat radius, int seg) {
    int segment = 4;
    for (GLfloat angle = M_PI/8; angle <= 2.0f * M_PI; angle += M_PI/4) {
        GLfloat x = xCenter + radius * cos(angle);
        GLfloat y = yCenter + radius * sin(angle);
        DrawEightCircleSegments2(x, y, 128, segment);
        if(segment++ == 7) segment = 0;
     //       break;
    }
}

void DrawCircleSegmentInround(int segment) {
    glColor3f(1, 1, 1);
   // DrawEightCircleSegments2(1920, 1080, 128.0f, 4);
    DrawCircle3(1920/2, 1080/2, 256-20, 4);
}

void funktion5(void) {
    DrawCircleSegmentInround(0);
}

void drawYinYang(void) {
 DrawEightCircleSegments3(1920/2, 1080/2, 128, 0);
//DrawYinYang(1920/2, 1080/2, 128);
}


// Add an array to store start times for each function
double start_time_functions[ANZAHL_FUNKTIONEN];


int main() {
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // GLFWwindow* window = glfwCreateWindow(800, 600, "glftTest 0.1", NULL, NULL);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "glftTest 0.1", glfwGetPrimaryMonitor(), NULL);

    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // GLFW-Callback-Funktionen setzen
    glfwSetCharCallback(window, handleCharInput);
    glfwSetKeyCallback(window, handleKeyboardInput);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    simpsonsfont = glftLoadFont("./fonts/Simpsonfont.ttf", 96);
    dejavu = glftLoadFont("./fonts/Guru Meditation NBP Regular.ttf", 12);
    arial = glftLoadFont("./fonts/Arial.ttf", 24);
    matrix = glftLoadFont("./fonts/Simpsonfont.ttf", 16);


// In the main function, initialize start_time_functions before the loop
for (int i = 0; i < ANZAHL_FUNKTIONEN; i++) {
  start_time_functions[i] = glfwGetTime();
}

    setfont();


    while (!glfwWindowShouldClose(window) && !gpause) {
        processInput(window);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int win_width, win_height;
        glfwGetFramebufferSize(window, &win_width, &win_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, win_width, win_height, 0, 0, 1);
        glMatrixMode(GL_MODELVIEW);

// Inside the main loop, after drawing the current function:
// Check if the current function's laufzeit is over
if (glfwGetTime() - start_time_functions[current_funktion_index] >= funktionen[current_funktion_index].laufzeit ) {
  // Move to the next function or loop back to the first one
  current_funktion_index = (current_funktion_index + 1) % ANZAHL_FUNKTIONEN;
  start_time_functions[current_funktion_index] = glfwGetTime();  // Reset start time for the new function
}

funktionen[current_funktion_index].funktion();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

// TODO CLEAN
//    simpsonsfont = glftLoadFont("./fonts/Simpsonfont.ttf", 16);
//    dejavu = glftLoadFont("./fonts/Guru Meditation NBP Regular.ttf", 16);
//    arial = glftLoadFont("./fonts/Arial.ttf", 16);
//    matrix = glftLoadFont("./fonts/Simpsonfont.ttf", 16);

    glftCleanup();
    glfwTerminate();
    return 0;
}
