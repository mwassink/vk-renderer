#include "../device.h"
#include "../renderer.h"

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE prevInstance,
                     LPSTR commandLine,
                     int showCode){
    Renderer vkRenderer;
    vkRenderer.Init();
    vkRenderer.InitBasicRender();
    auto model = vkRenderer.LoadModelObj("barrel/barrel.obj", "barrel/img.png");
    Vector3 r = Vector3(1,1,1);
    r.normalize();

    Matrix3 rm = Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    Quaternion q;
    q.SetRotation(rm);

    model.rotation = q;
    Vector4 pos = Vector4(1, 1, 1, 1);
    Vector4 color = Vector4(1, 1, 1, 1);
    Light l;

    l.lightData.lightColor = color;
    l.lightData.position = pos;
    l.lightData.power = 10.0f;
    l.lightData.shininess = 1.0f;
    
    




}