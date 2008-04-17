#ifndef RENDERER_H 
#define RENDERER_H 


#include "defines.h"
#include "CRoom.h"
#include "CRoomManager.h"
#include "CConfigurator.h"
#include "CFrustum.h"

class QFont;

//#define DIST_Z	2	/* the distance between 2 rooms */
#define BASE_Z  -12	/* the distance to the "camera" */
#define ROOM_SIZE 0.45f	/* the size of the rooms walls */

#define MAXHITS 200


// temporary storage for a billboard text
class Billboard {
	
public:
	Billboard(GLfloat _x, GLfloat _y, GLfloat _z, QString _text):x(_x),y(_y),z(_z),text(_text) {} 

	GLfloat 	x;
	GLfloat 	y;
	GLfloat 	z;
	QString text;
};

class RendererWidget : public QGLWidget
{
    Q_OBJECT
    
    GLfloat       colour[4];
    GLuint        global_list;
    int           curx;
    int           cury;
    int           curz;			/* current rooms position */ 
    CFrustum       frustum;
//    QFont         textFont;
    
    int           lowerZ;
    int           upperZ;
    
    QList< Billboard * > billboards;

    unsigned int last_drawn_marker;
    unsigned int last_drawn_trail;

    GLuint selectBuf[MAXHITS];
    
    void glDrawMarkers();
    void drawMarker(int, int, int, int);
    void glDrawRoom(CRoom *p);
    void glDrawCSquare(CSquare *p, int renderingMode);
    
    void setupViewingModel(  int width, int height  );
    void renderPickupObjects();  
    void renderPickupRoom(CRoom *p);


    // this sets curx, cury, curz based on hrm internal rules
    void setupNewBaseCoordinates();

    void draw();
    void drawCone();

public:
    GLfloat       angleY;
    GLfloat       angleX;
    GLfloat       angleZ;
    float         userX;
    float         userY;
    float         userZ;		/* additional shift added by user */
    int           userLayerShift;       // this affect curz.
    
    int           current_plane_z;        
    GLuint        basic_gllist;
    
    bool          redraw;       

    int           rooms_drawn_csquare;
    int           rooms_drawn_total;

    unsigned int deletedRoom;

    
    RendererWidget( QWidget *parent = 0);
    
    
    
    
    void initializeGL();
    void resizeGL( int width, int height );
    void changeUserLayerShift(int byValue) { userLayerShift += byValue; curz += byValue; }
    
    bool doSelect(QPoint pos, unsigned int &id);
    void centerOnRoom(unsigned int id);

    
public slots:
	void paintGL();
    void display(void);
};



#endif
