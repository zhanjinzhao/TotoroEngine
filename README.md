# TotoroEngine
TotoroEngine is a toy 3D game engine using DirectX 12.

# Build and Run
1. Download FBX SDK VS2019: https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2
2. Install FBX SDK, then copy "include" and "lib" folders to TotoroEngine\Engine\ThirdParty\FBX_SDK\
3. Open TotoroEngine.sln with Visual Studio 2019.
4. Choose a sample project (such as "Sample-PBR"), set it as startup project, build project.
5. Copy the libfbxsdk.dll (from TotoroEngine\Engine\ThirdParty\FBX_SDK\lib\vs2019\x64\debug) to the debug folder of sample project (such as TotoroEngine\Samples\Sample-PBR\Binaries\x64\Debug).
6. Run.

# Features
## Basis
* D3D12 Memory Allocation
* D3D12 Descriptor Management
* D3D12 Resource Binding

## Rendering
* PBR (Physically Based Rendering)
  ![](Screenshots/PBR_Cyborg.png)
  
  ![](Screenshots/PBR_Helmet.png)
  
  ![](Screenshots/PBR_Sphere.png)
  
  
  
* TBDR (Tile-Based Deferred Rendering)
  ![](Screenshots/TBDR.png)
  
  
  
* Light
  * DirectionalLight
  * PointLight
  ![](Screenshots/PointLight.png)
  
  
  * SpotLight
  ![](Screenshots/SpotLight.png) 
  
  
  * AreaLight(LTC)
  ![](Screenshots/AreaLight.png)
  
  
  
* PCSS (Percentage-Closer Soft Shadows)
  ![](Screenshots/PCSS.png)
  
  
  
* VSM (Variance Shadow Mapping)
  ![](Screenshots/VSM.png)
  
  
  
* SDF
  ![](Screenshots/SDF.png)
  
  
  
* SDF soft shadow
  ![](Screenshots/SDF_Shadow.png)
  
  
  
* SSAO (Screen Space Ambient Occlusion)
  ![](Screenshots/SSAO_Off.png)
  
  ![](Screenshots/SSAO_On.png)
  
  
  
* SSR (Screen Space Reflection)
  ![](Screenshots/SSR.png)
  
  
  
* TAA (Temporal Anti-Aliasing)
  ![](Screenshots/TAA_Off.png)
  
  ![](Screenshots/TAA_On.png)
  
* PostProcess
  * ToneMapping
  
# Document
https://www.zhihu.com/column/c_1434895110592552960 (Chinese)
  