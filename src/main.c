/*
 * THE LAST SHIFT: MALL AFTER DARK
 * First playable prototype - low-poly 3D SDL2 software renderer.
 * Target: Linux ARM handhelds / ArkOS PortMaster (build still required).
 */
#include <SDL2/SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W 320
#define H 240
#define PI 3.14159265358979323846f
#define MAX_TRIS 4096

typedef struct { float x,y,z; } V3;
typedef struct { V3 a,b,c; uint32_t color; } Tri;
typedef struct { V3 p; float yaw; } Camera;

static uint32_t pixels[W*H];
static float depthbuf[W*H];
static Tri tris[MAX_TRIS];
static int tri_count=0;
static SDL_Window *window=NULL;
static SDL_Renderer *renderer=NULL;
static SDL_Texture *texture=NULL;
static Camera cam={0.0f,0.0f,0.0f,0.0f};
static float guard_x=0.0f, guard_z=9.0f;
static int guard_alert=0, game_over=0;
static float move_speed=2.5f;
static const uint32_t C_FLOOR=0xff292a30, C_WALL=0xff42434b,
 C_DARK=0xff181a20, C_LIGHT=0xffa9a69a, C_RED=0xff6b2028,
 C_BLUE=0xff26364b, C_WHITE=0xffb8b5a8, C_GUARD=0xff17171b;

static V3 v3(float x,float y,float z){ V3 v={x,y,z}; return v; }
static V3 sub(V3 a,V3 b){ return v3(a.x-b.x,a.y-b.y,a.z-b.z); }
static V3 cross(V3 a,V3 b){ return v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x); }
static float dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
static V3 norm(V3 a){ float l=sqrtf(dot(a,a)); return l>0.0001f?v3(a.x/l,a.y/l,a.z/l):a; }
static uint32_t shade(uint32_t c,float k){
    int r=(c>>16)&255,g=(c>>8)&255,b=c&255;
    r=(int)(r*k);g=(int)(g*k);b=(int)(b*k);
    if(r>255)r=255;if(g>255)g=255;if(b>255)b=255;
    return 0xff000000u|((uint32_t)r<<16)|((uint32_t)g<<8)|(uint32_t)b;
}
static void addtri(V3 a,V3 b,V3 c,uint32_t color){
    if(tri_count<MAX_TRIS) tris[tri_count++]=(Tri){a,b,c,color};
}
static void quad(V3 a,V3 b,V3 c,V3 d,uint32_t col){
    addtri(a,b,c,col); addtri(a,c,d,col);
}
static void cube(float x,float y,float z,float sx,float sy,float sz,uint32_t col){
    float x0=x-sx/2,x1=x+sx/2,y0=y-sy/2,y1=y+sy/2,z0=z-sz/2,z1=z+sz/2;
    V3 a=v3(x0,y0,z0),b=v3(x1,y0,z0),c=v3(x1,y1,z0),d=v3(x0,y1,z0);
    V3 e=v3(x0,y0,z1),f=v3(x1,y0,z1),g=v3(x1,y1,z1),h=v3(x0,y1,z1);
    quad(a,b,c,d,shade(col,0.82f)); quad(f,e,h,g,shade(col,0.72f));
    quad(e,a,d,h,shade(col,0.62f)); quad(b,f,g,c,shade(col,0.95f));
    quad(d,c,g,h,shade(col,1.0f)); quad(e,f,b,a,shade(col,0.55f));
}
static void build_scene(void){
    tri_count=0;
    /* Main mall corridor: player begins in the security office, facing the mall. */
    quad(v3(-5,0,-2.35f),v3(5,0,-2.35f),v3(5,0,24),v3(-5,0,24),C_FLOOR);
    quad(v3(-5,4,-2.35f),v3(-5,4,24),v3(5,4,24),v3(5,4,-2.35f),C_DARK);
    quad(v3(-5,0,-2.35f),v3(-5,4,-2.35f),v3(-5,4,24),v3(-5,0,24),C_WALL);
    quad(v3(5,0,-2.35f),v3(5,0,24),v3(5,4,24),v3(5,4,-2.35f),C_WALL);
    /* Far wall and dark shopfront recesses. */
    quad(v3(-5,0,24),v3(-5,4,24),v3(5,4,24),v3(5,0,24),C_DARK);
    for(int z=2;z<21;z+=5){
        cube(-4.94f,1.6f,(float)z,0.12f,2.5f,3.0f,C_DARK);
        cube( 4.94f,1.6f,(float)z,0.12f,2.5f,3.0f,C_DARK);
        cube(-4.86f,3.85f,(float)z,0.18f,0.12f,2.7f,C_LIGHT);
        cube( 4.86f,3.85f,(float)z,0.18f,0.12f,2.7f,C_LIGHT);
    }
    /* Security office desk, monitors, and doorway frame. */
    cube(0,0.62f,-1.1f,2.7f,0.18f,0.9f,C_RED);
    cube(-0.55f,1.12f,-1.42f,0.75f,0.72f,0.10f,C_DARK);
    cube(0.48f,1.12f,-1.42f,0.75f,0.72f,0.10f,C_DARK);
    cube(-0.55f,1.12f,-1.35f,0.62f,0.48f,0.025f,C_BLUE);
    cube(0.48f,1.12f,-1.35f,0.62f,0.48f,0.025f,C_BLUE);
    cube(-1.45f,1.0f,-0.4f,0.12f,2.0f,0.18f,C_LIGHT);
    cube( 1.45f,1.0f,-0.4f,0.12f,2.0f,0.18f,C_LIGHT);
    cube(0,2.0f,-0.4f,3.0f,0.12f,0.18f,C_LIGHT);
    /* Ceiling lights. */
    for(int z=1;z<22;z+=4) cube(0,3.92f,(float)z,1.5f,0.035f,0.65f,C_LIGHT);
    /* Night Guard: low-poly blocky humanoid silhouette at end of hall. */
    cube(guard_x,1.15f,guard_z,0.72f,1.75f,0.42f,C_GUARD);
    cube(guard_x,2.18f,guard_z,0.55f,0.55f,0.48f,C_GUARD);
    cube(guard_x-0.14f,2.22f,guard_z-0.25f,0.07f,0.07f,0.025f,C_RED);
    cube(guard_x+0.14f,2.22f,guard_z-0.25f,0.07f,0.07f,0.025f,C_RED);
    cube(guard_x-0.22f,0.38f,guard_z,0.18f,0.75f,0.22f,C_GUARD);
    cube(guard_x+0.22f,0.38f,guard_z,0.18f,0.75f,0.22f,C_GUARD);
}
typedef struct { float x,y,z; } P2;
static int project(V3 p,P2 *o,float *zout){
    float dx=p.x-cam.p.x,dy=p.y-cam.p.y,dz=p.z-cam.p.z;
    float cs=cosf(cam.yaw),sn=sinf(cam.yaw);
    float x=dx*cs-dz*sn, z=dx*sn+dz*cs;
    if(z<0.12f)return 0;
    o->x=(x/z)*170.0f+W*0.5f;
    o->y=H*0.5f-((dy-1.35f)/z)*170.0f;
    *zout=z; return 1;
}
static void pixel(int x,int y,float z,uint32_t col){
    if(x<0||x>=W||y<0||y>=H)return;
    int i=y*W+x;if(z<depthbuf[i]){depthbuf[i]=z;pixels[i]=col;}
}
static void raster(Tri t){
    P2 a,b,c;float za,zb,zc;
    if(!project(t.a,&a,&za)||!project(t.b,&b,&zb)||!project(t.c,&c,&zc))return;
    float minxf=fminf(a.x,fminf(b.x,c.x)),maxxf=fmaxf(a.x,fmaxf(b.x,c.x));
    float minyf=fminf(a.y,fminf(b.y,c.y)),maxyf=fmaxf(a.y,fmaxf(b.y,c.y));
    int minx=(int)fmaxf(0,floorf(minxf)),maxx=(int)fminf(W-1,ceilf(maxxf));
    int miny=(int)fmaxf(0,floorf(minyf)),maxy=(int)fminf(H-1,ceilf(maxyf));
    float area=(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);if(fabsf(area)<0.001f)return;
    for(int y=miny;y<=maxy;y++)for(int x=minx;x<=maxx;x++){
        float px=x+0.5f,py=y+0.5f;
        float u=((b.x-px)*(c.y-py)-(b.y-py)*(c.x-px))/area;
        float v=((c.x-px)*(a.y-py)-(c.y-py)*(a.x-px))/area;
        float w=1-u-v;if(u>=0&&v>=0&&w>=0){
            float iz=u/za+v/zb+w/zc;if(iz>0)pixel(x,y,1.0f/iz,t.color);
        }
    }
}
static void render(void){
    for(int i=0;i<W*H;i++){pixels[i]=0xff08090e;depthbuf[i]=1e9f;}
    build_scene();
    for(int i=0;i<tri_count;i++)raster(tris[i]);
    SDL_UpdateTexture(texture,NULL,pixels,W*sizeof(uint32_t));
    SDL_RenderClear(renderer);SDL_RenderCopy(renderer,texture,NULL,NULL);SDL_RenderPresent(renderer);
}
static float clampf(float x,float a,float b){return x<a?a:(x>b?b:x);}
static void update(float dt,const Uint8 *keys,SDL_GameController *pad){
    float forward=0,side=0,turn=0;
    if(keys[SDL_SCANCODE_W]||keys[SDL_SCANCODE_UP])forward+=1;
    if(keys[SDL_SCANCODE_S]||keys[SDL_SCANCODE_DOWN])forward-=1;
    if(keys[SDL_SCANCODE_A])side-=1;if(keys[SDL_SCANCODE_D])side+=1;
    if(keys[SDL_SCANCODE_LEFT])turn-=1;if(keys[SDL_SCANCODE_RIGHT])turn+=1;
    if(pad){
        float ly=SDL_GameControllerGetAxis(pad,SDL_CONTROLLER_AXIS_LEFTY)/32768.0f;
        float lx=SDL_GameControllerGetAxis(pad,SDL_CONTROLLER_AXIS_LEFTX)/32768.0f;
        float rx=SDL_GameControllerGetAxis(pad,SDL_CONTROLLER_AXIS_RIGHTX)/32768.0f;
        if(fabsf(ly)>0.18f)forward=-ly;if(fabsf(lx)>0.18f)side=lx;
        if(fabsf(rx)>0.18f)turn=rx;
    }
    cam.yaw+=turn*dt*2.0f;
    float cs=cosf(cam.yaw),sn=sinf(cam.yaw);
    cam.p.x+=(sn*forward+cs*side)*move_speed*dt;
    cam.p.z+=(cs*forward-sn*side)*move_speed*dt;
    cam.p.x=clampf(cam.p.x,-4.25f,4.25f);cam.p.z=clampf(cam.p.z,-2.5f,22.5f);
    float dx=cam.p.x-guard_x,dz=cam.p.z-guard_z;
    float dist=sqrtf(dx*dx+dz*dz);
    if(dist<10.0f)guard_alert=1;
    if(guard_alert&&dist>0.65f){
        guard_x+=(dx/dist)*dt*0.78f;guard_z+=(dz/dist)*dt*0.78f;
    }
    if(dist<0.72f)game_over=1;
}
int main(int argc,char **argv){
    (void)argc;(void)argv;
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_AUDIO)<0)return 1;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
    window=SDL_CreateWindow("THE LAST SHIFT",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        W*2,H*2,SDL_WINDOW_FULLSCREEN_DESKTOP);
    if(!window)window=SDL_CreateWindow("THE LAST SHIFT",0,0,W*2,H*2,0);
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
    texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,W,H);
    SDL_GameController *pad=NULL;
    for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_IsGameController(i)){pad=SDL_GameControllerOpen(i);if(pad)break;}
    cam.p=v3(0,0,-2.5f);cam.yaw=0;
    uint32_t last=SDL_GetTicks();int running=1;
    while(running){
        SDL_Event e;while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT)running=0;
            if(e.type==SDL_KEYDOWN&&e.key.keysym.sym==SDLK_ESCAPE)running=0;
            if(e.type==SDL_CONTROLLERBUTTONDOWN&&e.cbutton.button==SDL_CONTROLLER_BUTTON_START)running=0;
        }
        uint32_t now=SDL_GetTicks();float dt=(now-last)/1000.0f;last=now;if(dt>0.05f)dt=0.05f;
        const Uint8 *keys=SDL_GetKeyboardState(NULL);
        if(!game_over)update(dt,keys,pad);
        render();
        if(game_over){
            SDL_SetWindowTitle(window,"CAUGHT. THE NIGHT GUARD FOUND YOU.  -  START/ESC TO QUIT");
        }else if(guard_alert)SDL_SetWindowTitle(window,"THE LAST SHIFT  -  HE'S COMING. RUN!");
        else SDL_SetWindowTitle(window,"THE LAST SHIFT  -  SECURITY OFFICE / MALL");
        SDL_Delay(16);
    }
    if(pad)SDL_GameControllerClose(pad);
    SDL_DestroyTexture(texture);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
    return 0;
}
