#include "../device.h"
#include "../basicrenderer.h"

#define PI 3.14159

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE prevInstance,
                     LPSTR commandLine,
                     int showCode){
    BasicRenderer vkRenderer;
    vkRenderer.Init();
    vkRenderer.InitBasicRender(10000);
    BasicModelFiles files = {"tests/barrel/barrel.obj", "tests/barrel/img.png"};
    auto model = vkRenderer.AddBasicModel(files);
    
    Vector3 r = Vector3(1,1,1);
    r.normalize();

    Matrix3 rm = Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);
#if 0    
    Quaternion q;
    q.SetRotation(rm);
    model.rotation = q;
    
#endif
    
    Vector4 pos = Vector4(1, 1, 1, 1);
    Vector4 color = Vector4(1, 1, 1, 1);
    BasicLightData lightData;

    lightData.lightColor = color;
    lightData.position = pos;
    lightData.power = 10.0f;
    lightData.shininess = 1.0f;
    CoordinateSpace worldSpace;
    CoordinateSpace oSpace;
    oSpace.origin = Vector3(0, 0, -5);
    Matrix4 mvp = ModelViewProjection(oSpace, worldSpace, PI/4, 16.0f/9.0f,0.5f, 10.0f );
    Matrix4 mv = ModelView(oSpace, worldSpace);
    Matrix3 normalTransform(1, 0, 0, 0, 1, 0, 0, 0, 1);
    normalTransform = NormalTransform(normalTransform);
    model.matrices.modelViewProjection = mvp;
    model.matrices.modelView = mv;
    model.matrices.normalMatrix = normalTransform;
    
    
    
    

    BasicFlatScene scene = vkRenderer.SimpleScene(&model, 1, &lightData, 1);

    bool run = true;
    
    vkRenderer.ctx.pform.ShowWindow();
    for (;vkRenderer.Runnable();) {

        vkRenderer.DrawBasicFlatScene(&scene);
        vkRenderer.WindowUpdates();
    }


    




}
