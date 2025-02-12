# 3DRenderer
## Instalacja
```
git clone --recursive https://github.com/kasrow12/3DRenderer.git
cd 3DRenderer
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release && build\Release\My3DRenderer.exe
```

## Flaga na wietrze (p≥at Beziera) (Tessellation Shader)
DziÍki zastosowaniu shaderÛw teselacji, moøliwe jest wygenerowanie dodatkowych wierzcho≥kÛw, ktÛre pozwalajπ na zwiÍkszenie rozdzielczoúci powierzchni.

Do teselacji potrzebne nam sπ:
- [Control Shader](Assets/Shaders/control.tcs) - kontroluje iloúÊ generowanych wierzcho≥kÛw, parametr tessLevel okreúla iloúÊ generowanych dodatkowych wierzcho≥kÛw na krawÍdü
- [Evaluation Shader](Assets/Shaders/evaluation.tes) - oblicza pozycje nowych wierzcho≥kÛw i wylicza wartoúci powierzchni Beziera

## Zmiana sk≥adowej zwierciadlanej Phong/Blinn
We fragment shaderze (`Assets/Shaders/fragment.fs`) moøna zmieniÊ sk≥adowπ zwierciadlanπ z Phonga na Blinna, korzystajπc z uniforma `blinn`.
W aplikacji moøna zmieniaÊ wartoúÊ zmiennej `blinn` za pomocπ klawisza `B`.
[èrÛd≥o](https://learnopengl.com/Advanced-Lighting/Advanced-Lighting)
```glsl
float spec = 0.0;
if (blinn)
{
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
}
else
{
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
}
```