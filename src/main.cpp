#include <GL/freeglut.h>
#include <GL/glext.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

constexpr float PI = 3.14159265358979323846f;

struct Vec3 {
    float x{}, y{}, z{};
    Vec3 operator+(const Vec3& b) const { return {x + b.x, y + b.y, z + b.z}; }
    Vec3 operator-(const Vec3& b) const { return {x - b.x, y - b.y, z - b.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
};

float length(const Vec3& v) { return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
Vec3 normalized(const Vec3& v) { const float n = length(v); return n > 0.0001f ? v * (1.0f/n) : Vec3{}; }

struct Box { Vec3 center, half; };

Vec3 dronePos{0.0f, 2.5f, 13.0f};
float droneYaw = 0.0f;
float moveSpeed = 6.0f;
float propellerAngle = 0.0f;
float sceneTime = 0.0f;
float dronePitch = 0.0f;
float droneRoll = 0.0f;
float cameraYaw = 20.0f;
float cameraPitch = 18.0f;
int cameraMode = 0;
bool autoPilot = false;
bool showGrid = true;
bool showHelp = true;
bool collisionFlash = false;
bool nightMode = false;
bool showFog = false;
bool mouseLookEnabled = true;
int collisionFrames = 0;
int windowWidth = 1280, windowHeight = 720;
int lastTimeMs = 0;
std::array<bool, 256> keys{};
std::array<bool, 256> specialKeys{};

const std::vector<Box> buildings = {
    {{-9, 3, 2}, {3, 3, 3}}, {{7, 4, -3}, {2.5f, 4, 3}},
    {{-3, 2.5f, -11}, {3, 2.5f, 2.5f}}, {{12, 2, 10}, {2, 2, 2.5f}},
    {{-14, 4.5f, -13}, {2.5f, 4.5f, 3}}
};

const std::vector<Vec3> trees = {
    {-15,0,7}, {-5,0,8}, {4,0,7}, {15,0,-7}, {2,0,-17},
    {-9,0,-18}, {17,0,2}, {-18,0,-4}, {9,0,16}
};

const std::vector<Vec3> route = {
    {0,3,13}, {-2,5,5}, {-1,7,-2}, {1,7,-8}, {6,6,-13},
    {13,6,-12}, {15,5,-2}, {11,6,7}, {3,5,13}, {0,3,13}
};
std::size_t waypoint = 1;

bool overlaps(const Box& a, const Box& b) {
    return std::abs(a.center.x-b.center.x) <= a.half.x+b.half.x &&
           std::abs(a.center.y-b.center.y) <= a.half.y+b.half.y &&
           std::abs(a.center.z-b.center.z) <= a.half.z+b.half.z;
}

bool collides(const Vec3& p) {
    if (p.y < 0.65f || p.y > 18.0f || std::abs(p.x) > 23.0f || std::abs(p.z) > 23.0f) return true;
    const Box drone{p, {0.85f, 0.35f, 0.85f}};
    for (const auto& building : buildings) if (overlaps(drone, building)) return true;
    for (const auto& tree : trees) {
        Box crown{{tree.x, 2.8f, tree.z}, {1.45f, 2.4f, 1.45f}};
        if (overlaps(drone, crown)) return true;
    }
    return false;
}

void cube(float sx, float sy, float sz) {
    glPushMatrix(); glScalef(sx, sy, sz); glutSolidCube(1.0); glPopMatrix();
}

void disc(float radius, int slices = 32) {
    glBegin(GL_TRIANGLE_FAN); glVertex3f(0,0,0);
    for (int i=0;i<=slices;++i) { float a=2*PI*i/slices; glVertex3f(std::cos(a)*radius,0,std::sin(a)*radius); }
    glEnd();
}

void drawGround() {
    glDisable(GL_LIGHTING);
    glColor3f(0.12f, 0.30f, 0.15f);
    glBegin(GL_QUADS); glVertex3f(-25,0,-25); glVertex3f(-25,0,25); glVertex3f(25,0,25); glVertex3f(25,0,-25); glEnd();
    if (showGrid) {
        glColor3f(0.22f, 0.42f, 0.23f); glBegin(GL_LINES);
        for (int i=-25; i<=25; ++i) { glVertex3f((float)i,0.01f,-25); glVertex3f((float)i,0.01f,25); glVertex3f(-25,0.01f,(float)i); glVertex3f(25,0.01f,(float)i); }
        glEnd();
    }
    glEnable(GL_LIGHTING);
    glColor3f(0.16f,0.17f,0.18f); glPushMatrix(); glTranslatef(0,0.025f,0); cube(6,0.04f,50); glPopMatrix();
    glDisable(GL_LIGHTING); glColor3f(0.95f,0.82f,0.18f);
    for (int z=-23;z<24;z+=4) { glBegin(GL_QUADS); glVertex3f(-0.08f,0.055f,(float)z); glVertex3f(0.08f,0.055f,(float)z); glVertex3f(0.08f,0.055f,z+2.0f); glVertex3f(-0.08f,0.055f,z+2.0f); glEnd(); }
    glColor3f(0.22f,0.24f,0.25f); glBegin(GL_TRIANGLE_FAN); glVertex3f(-17,0.06f,17); for(int i=0;i<=40;++i){float a=2*PI*i/40;glVertex3f(-17+3.2f*std::cos(a),0.06f,17+3.2f*std::sin(a));} glEnd();
    glColor3f(0.95f,0.95f,0.95f); glLineWidth(5); glBegin(GL_LINES); glVertex3f(-18.1f,0.075f,15.5f);glVertex3f(-18.1f,0.075f,18.5f);glVertex3f(-15.9f,0.075f,15.5f);glVertex3f(-15.9f,0.075f,18.5f);glVertex3f(-18.1f,0.075f,17);glVertex3f(-15.9f,0.075f,17);glEnd();
    glEnable(GL_LIGHTING);
}

void drawBuilding(const Box& b, int i) {
    glPushMatrix(); glTranslatef(b.center.x,b.center.y,b.center.z);
    glColor3f(0.38f+0.04f*(i%3),0.42f,0.48f); cube(b.half.x*2,b.half.y*2,b.half.z*2);
    glColor3f(0.12f,0.25f,0.34f);
    const float front = b.half.z + 0.01f;
    for (float y=-b.half.y+1.0f; y<b.half.y; y+=1.5f)
        for (float x=-b.half.x+0.7f; x<b.half.x; x+=1.3f) {
            glPushMatrix(); glTranslatef(x,y,front); cube(0.55f,0.65f,0.03f); glPopMatrix();
        }
    glPopMatrix();
    if (b.half.y > 3.0f) {
        float pulse=0.55f+0.45f*std::sin(sceneTime*5.0f+i);
        glPushMatrix(); glTranslatef(b.center.x,b.center.y+b.half.y+0.22f,b.center.z);
        glColor3f(1.0f,pulse*0.12f,0.05f); glutSolidSphere(0.18,12,8); glPopMatrix();
    }
}

void drawTree(const Vec3& p) {
    glPushMatrix(); glTranslatef(p.x,0,p.z);
    glColor3f(0.35f,0.18f,0.07f); glPushMatrix(); glTranslatef(0,1.25f,0); cube(0.45f,2.5f,0.45f); glPopMatrix();
    glColor3f(0.08f,0.42f,0.13f); glPushMatrix(); glTranslatef(0,3.4f,0); glRotatef(std::sin(sceneTime*1.7f+p.x)*2.5f,0,0,1); glutSolidCone(1.7,3.8,16,5); glPopMatrix();
    glPopMatrix();
}

void drawCloud(float x,float y,float z,float scale) {
    glPushMatrix(); glTranslatef(x,y,z); glScalef(scale,scale,scale);
    if(nightMode) glColor3f(0.28f,0.31f,0.38f); else glColor3f(0.92f,0.94f,1.0f);
    const float parts[5][4]={{0,0,0,1.1f},{1.0f,0.1f,0,0.8f},{-1.0f,0,0,0.75f},{0.4f,0.55f,0,0.75f},{-0.45f,0.45f,0,0.8f}};
    for(auto& q:parts){glPushMatrix();glTranslatef(q[0],q[1],q[2]);glScalef(1.35f,0.65f,0.8f);glutSolidSphere(q[3],14,9);glPopMatrix();}
    glPopMatrix();
}

void drawCars() {
    for(int i=0;i<3;++i){
        float z=std::fmod(sceneTime*(3.2f+i*0.35f)+i*16.0f,54.0f)-27.0f;
        float x=(i%2==0)?-1.55f:1.55f;
        glPushMatrix();glTranslatef(x,0.32f,z);if(i%2)glRotatef(180,0,1,0);
        glColor3f(i==0?0.9f:0.12f,i==1?0.65f:0.15f,i==2?0.85f:0.2f);cube(1.25f,0.35f,2.1f);
        glColor3f(0.12f,0.2f,0.28f);glPushMatrix();glTranslatef(0,0.28f,-0.1f);cube(1.0f,0.35f,1.0f);glPopMatrix();glPopMatrix();
    }
}

void drawDrone() {
    glPushMatrix(); glTranslatef(dronePos.x,dronePos.y+std::sin(sceneTime*3.0f)*0.035f,dronePos.z); glRotatef(droneYaw,0,1,0); glRotatef(dronePitch,1,0,0); glRotatef(droneRoll,0,0,1);
    if (collisionFlash) glColor3f(1.0f,0.12f,0.08f); else glColor3f(0.12f,0.22f,0.30f);
    cube(1.4f,0.38f,1.0f);
    glColor3f(0.04f,0.68f,0.88f); glPushMatrix(); glTranslatef(0,0.23f,-0.05f); cube(0.75f,0.25f,0.65f); glPopMatrix();
    glColor3f(0.15f,0.16f,0.17f); cube(3.4f,0.12f,0.16f); cube(0.16f,0.12f,3.4f);
    const float arms[4][2]={{-1.65f,0},{1.65f,0},{0,-1.65f},{0,1.65f}};
    for (auto& arm : arms) {
        glPushMatrix(); glTranslatef(arm[0],0.08f,arm[1]);
        glColor3f(0.07f,0.07f,0.08f); glutSolidSphere(0.24,12,8);
        glRotatef(propellerAngle,0,1,0); glColor3f(0.72f,0.78f,0.82f); cube(1.45f,0.035f,0.13f);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); glColor4f(0.65f,0.85f,1.0f,0.18f); disc(0.82f); glDisable(GL_BLEND);
        glPopMatrix();
    }
    glColor3f(0.08f,0.08f,0.09f);
    for (float x : {-0.48f,0.48f}) for (float z : {-0.3f,0.3f}) { glPushMatrix(); glTranslatef(x,-0.34f,z); cube(0.09f,0.55f,0.09f); glPopMatrix(); }
    float blink=0.45f+0.55f*(std::sin(sceneTime*7.0f)>0);
    glColor3f(blink,0.02f,0.02f);glPushMatrix();glTranslatef(-1.68f,0.1f,0);glutSolidSphere(0.12,10,7);glPopMatrix();
    glColor3f(0.02f,blink,0.08f);glPushMatrix();glTranslatef(1.68f,0.1f,0);glutSolidSphere(0.12,10,7);glPopMatrix();
    glPopMatrix();
}

void drawDroneShadow() {
    float alpha=std::max(0.08f,0.33f-dronePos.y*0.012f);
    glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glColor4f(0,0,0,alpha);
    glPushMatrix();glTranslatef(dronePos.x,0.08f,dronePos.z);glRotatef(droneYaw,0,1,0);glScalef(1.0f,1.0f,0.55f);disc(2.0f);glPopMatrix();
    glDisable(GL_BLEND);glEnable(GL_LIGHTING);
}

void drawRoute() {
    glDisable(GL_LIGHTING); glColor3f(0.1f,0.9f,1.0f); glLineWidth(2.5f);
    glBegin(GL_LINE_STRIP); for (const auto& p : route) glVertex3f(p.x,p.y,p.z); glEnd();
    glPointSize(8); glBegin(GL_POINTS); for (const auto& p : route) glVertex3f(p.x,p.y,p.z); glEnd();
    glEnable(GL_LIGHTING);
}

void setCamera() {
    const float yaw = droneYaw * PI/180.0f;
    if (cameraMode == 1) {
        Vec3 eye = dronePos + Vec3{std::sin(yaw)*0.2f,0.22f,-std::cos(yaw)*0.2f};
        Vec3 target = eye + Vec3{-std::sin(yaw)*10.0f,0,-std::cos(yaw)*10.0f};
        gluLookAt(eye.x,eye.y,eye.z,target.x,target.y,target.z,0,1,0);
    } else if (cameraMode == 2) {
        gluLookAt(25,27,29, dronePos.x,dronePos.y,dronePos.z, 0,1,0);
    } else {
        float cy = (droneYaw+cameraYaw)*PI/180.0f, cp = cameraPitch*PI/180.0f;
        Vec3 eye = dronePos + Vec3{std::sin(cy)*std::cos(cp)*10.0f, 3.0f+std::sin(cp)*10.0f, std::cos(cy)*std::cos(cp)*10.0f};
        gluLookAt(eye.x,eye.y,eye.z,dronePos.x,dronePos.y,dronePos.z,0,1,0);
    }
}

void text(int x, int y, const std::string& value) {
    glRasterPos2i(x,y); for (unsigned char c : value) glutBitmapCharacter(GLUT_BITMAP_9_BY_15,c);
}

void drawHud() {
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0,windowWidth,0,windowHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glColor3f(0.85f,0.96f,1.0f);
    std::ostringstream line; line<<std::fixed<<std::setprecision(1)<<"ALT "<<dronePos.y<<" m   SPEED "<<moveSpeed<<"   CAMERA "<<(cameraMode==0?"CHASE":cameraMode==1?"COCKPIT":"OVERVIEW")<<"   AUTOPILOT "<<(autoPilot?"ON":"OFF")<<"   "<<(nightMode?"NIGHT":"DAY");
    text(18,windowHeight-28,line.str());
    if (collisionFlash) { glColor3f(1,0.25f,0.15f); text(windowWidth/2-90,windowHeight-55,"OBSTACLE - MOVEMENT BLOCKED"); }
    if (showHelp) { glColor3f(0.8f,0.86f,0.9f); text(18,48,"W/S forward  A/D strafe  R/F altitude  move touchpad/mouse to look"); text(18,27,"C camera  P autopilot  M release mouse  N day/night  V fog  H help"); }
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}

void display() {
    if(nightMode)glClearColor(0.025f,0.045f,0.11f,1);else glClearColor(0.46f,0.70f,0.90f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity(); setCamera();
    const GLfloat sun[]={12,22,8,1}; const GLfloat dayDiffuse[]={0.9f,0.88f,0.78f,1}; const GLfloat nightDiffuse[]={0.25f,0.3f,0.5f,1}; glLightfv(GL_LIGHT0,GL_POSITION,sun); glLightfv(GL_LIGHT0,GL_DIFFUSE,nightMode?nightDiffuse:dayDiffuse);
    drawGround(); for (std::size_t i=0;i<buildings.size();++i) drawBuilding(buildings[i],(int)i); for (const auto& tree:trees) drawTree(tree);
    drawCars();
    drawCloud(std::fmod(sceneTime*0.8f+15.0f,65.0f)-32.0f,15,-12,1.8f); drawCloud(std::fmod(sceneTime*0.5f+42.0f,70.0f)-35.0f,12,10,1.25f); drawCloud(std::fmod(sceneTime*0.65f+5.0f,68.0f)-34.0f,17,2,1.45f);
    drawRoute(); drawDroneShadow(); drawDrone(); drawHud(); glutSwapBuffers();
}

void tryMove(const Vec3& delta) {
    const Vec3 next=dronePos+delta;
    if (!collides(next)) dronePos=next; else { collisionFlash=true; collisionFrames=20; }
}

void update() {
    const int now=glutGet(GLUT_ELAPSED_TIME); float dt=std::min(0.05f,(now-lastTimeMs)/1000.0f); lastTimeMs=now;
    sceneTime+=dt;
    const float yaw=droneYaw*PI/180.0f; Vec3 forward{-std::sin(yaw),0,-std::cos(yaw)}, right{std::cos(yaw),0,-std::sin(yaw)};
    if (!autoPilot) {
        Vec3 move{};
        if(keys['w']||keys['W'])move=move+forward;
        if(keys['s']||keys['S'])move=move-forward;
        if(keys['d']||keys['D'])move=move+right;
        if(keys['a']||keys['A'])move=move-right;
        if(length(move)>0)tryMove(normalized(move)*(moveSpeed*dt));
        float targetPitch=((keys['w']||keys['W'])?10.0f:0)-((keys['s']||keys['S'])?10.0f:0);
        float targetRoll=((keys['a']||keys['A'])?12.0f:0)-((keys['d']||keys['D'])?12.0f:0);
        dronePitch+=(targetPitch-dronePitch)*std::min(1.0f,dt*5.0f);
        droneRoll+=(targetRoll-droneRoll)*std::min(1.0f,dt*5.0f);
        if(keys['r']||keys['R'])tryMove({0,moveSpeed*0.7f*dt,0});
        if(keys['f']||keys['F'])tryMove({0,-moveSpeed*0.7f*dt,0});
        if(keys['q']||keys['Q'])droneYaw+=75*dt;
        if(keys['e']||keys['E'])droneYaw-=75*dt;
    } else {
        Vec3 d=route[waypoint]-dronePos;
        if(length(d)<0.45f) waypoint=(waypoint+1)%route.size(); else { Vec3 n=normalized(d); tryMove(n*(moveSpeed*0.7f*dt)); droneYaw=std::atan2(-n.x,-n.z)*180.0f/PI; dronePitch+=(8.0f-dronePitch)*std::min(1.0f,dt*3.0f); droneRoll=std::sin(sceneTime*1.5f)*3.0f; }
    }
    if(specialKeys[GLUT_KEY_LEFT])cameraYaw-=65*dt;
    if(specialKeys[GLUT_KEY_RIGHT])cameraYaw+=65*dt;
    if(specialKeys[GLUT_KEY_UP])cameraPitch=std::min(65.0f,cameraPitch+45*dt);
    if(specialKeys[GLUT_KEY_DOWN])cameraPitch=std::max(-10.0f,cameraPitch-45*dt);
    propellerAngle=std::fmod(propellerAngle+900*dt,360.0f);
    if(collisionFrames>0)--collisionFrames; else collisionFlash=false;
    glutPostRedisplay();
}

void reshape(int w,int h) { windowWidth=std::max(1,w); windowHeight=std::max(1,h); glViewport(0,0,w,h); glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(60.0,(double)windowWidth/windowHeight,0.1,150.0); glMatrixMode(GL_MODELVIEW); }

void keyDown(unsigned char k,int,int) {
    keys[k]=true;
    if(k==27)std::exit(0);
    if(k=='c'||k=='C')cameraMode=(cameraMode+1)%3;
    if(k=='p'||k=='P')autoPilot=!autoPilot;
    if(k=='g'||k=='G')showGrid=!showGrid;
    if(k=='h'||k=='H')showHelp=!showHelp;
    if(k=='m'||k=='M') {
        mouseLookEnabled=!mouseLookEnabled;
        glutSetCursor(mouseLookEnabled?GLUT_CURSOR_NONE:GLUT_CURSOR_INHERIT);
        if(mouseLookEnabled) glutWarpPointer(windowWidth/2,windowHeight/2);
    }
    if(k=='n'||k=='N')nightMode=!nightMode;
    if(k=='v'||k=='V') {
        showFog=!showFog;
        if(showFog){GLfloat fogColor[]={0.55f,0.62f,0.68f,1};glFogfv(GL_FOG_COLOR,fogColor);glFogf(GL_FOG_DENSITY,0.018f);glFogi(GL_FOG_MODE,GL_EXP2);glEnable(GL_FOG);}
        else glDisable(GL_FOG);
    }
    if(k=='+'||k=='=')moveSpeed=std::min(15.0f,moveSpeed+1);
    if(k=='-'||k=='_')moveSpeed=std::max(2.0f,moveSpeed-1);
}
void keyUp(unsigned char k,int,int){keys[k]=false;}
void specialDown(int k,int,int){if(k>=0&&k<256)specialKeys[k]=true;}
void specialUp(int k,int,int){if(k>=0&&k<256)specialKeys[k]=false;}

void pointerLook(int x,int y) {
    if(!mouseLookEnabled) return;
    const int centerX=windowWidth/2, centerY=windowHeight/2;
    const int deltaX=x-centerX, deltaY=y-centerY;
    if(deltaX==0 && deltaY==0) return;
    droneYaw-=deltaX*0.32f;
    cameraPitch=std::clamp(cameraPitch-deltaY*0.20f,-10.0f,65.0f);
    droneRoll=std::clamp(-deltaX*0.35f,-12.0f,12.0f);
    glutWarpPointer(centerX,centerY);
}

int main(int argc,char** argv) {
    glutInit(&argc,argv); glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH|GLUT_MULTISAMPLE); glutInitWindowSize(windowWidth,windowHeight); glutCreateWindow("3D Drone Navigation Simulator");
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL); glEnable(GL_NORMALIZE); glEnable(GL_MULTISAMPLE);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE); glClearColor(0.46f,0.70f,0.90f,1);
    const GLfloat ambient[]={0.28f,0.28f,0.32f,1}; const GLfloat diffuse[]={0.9f,0.88f,0.78f,1}; glLightfv(GL_LIGHT0,GL_AMBIENT,ambient); glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
    glutDisplayFunc(display); glutReshapeFunc(reshape); glutKeyboardFunc(keyDown); glutKeyboardUpFunc(keyUp); glutSpecialFunc(specialDown); glutSpecialUpFunc(specialUp); glutPassiveMotionFunc(pointerLook); glutMotionFunc(pointerLook); glutSetCursor(GLUT_CURSOR_NONE); glutWarpPointer(windowWidth/2,windowHeight/2); glutIdleFunc(update);
    lastTimeMs=glutGet(GLUT_ELAPSED_TIME); glutMainLoop(); return 0;
}


