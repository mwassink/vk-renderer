#include "../device.h"
#include "../renderer.h"

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE prevInstance,
                     LPSTR commandLine,
                     int showCode){
    Renderer vkRenderer;
    vkRenderer.Init();
    vkRenderer.InitBasicRender(10000);
    BasicModelFiles files = {"tests/barrel/barrel.obj", "tests/barrel/img.png"};
    auto model = vkRenderer.AddBasicModel(files);
    Vector3 r = Vector3(1,1,1);
    r.normalize();

    Matrix3 rm = Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    Quaternion q;
    q.SetRotation(rm);

    model.rotation = q;
    Vector4 pos = Vector4(1, 1, 1, 1);
    Vector4 color = Vector4(1, 1, 1, 1);
    BasicLightData lightData;

    lightData.lightColor = color;
    lightData.position = pos;
    lightData.power = 10.0f;
    lightData.shininess = 1.0f;

    BasicFlatScene scene = vkRenderer.SimpleScene(&model, 1, &lightData, 1);

    bool run = true;
    
    vkRenderer.ctx.pform.ShowWindow();
    for (;vkRenderer.Runnable();) {

        vkRenderer.DrawBasicFlatScene(&scene);
        vkRenderer.WindowUpdates();
    }


    




}
