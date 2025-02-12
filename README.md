# 3DRenderer
## Instalacja
```
git clone --recursive https://github.com/kasrow12/3DRenderer.git
cd 3DRenderer
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release && build\Release\My3DRenderer.exe
```

## Flaga na wietrze (p�at Beziera) (Tessellation Shader)
Dzi�ki zastosowaniu shader�w teselacji, mo�liwe jest wygenerowanie dodatkowych wierzcho�k�w, kt�re pozwalaj� na zwi�kszenie rozdzielczo�ci powierzchni.

Do teselacji potrzebne nam s�:
- [Control Shader](Assets/Shaders/control.tcs) - kontroluje ilo�� generowanych wierzcho�k�w, parametr tessLevel okre�la ilo�� generowanych dodatkowych wierzcho�k�w na kraw�d�
- [Evaluation Shader](Assets/Shaders/evaluation.tes) - oblicza pozycje nowych wierzcho�k�w i wylicza warto�ci powierzchni Beziera

## Zmiana sk�adowej zwierciadlanej Phong/Blinn
We fragment shaderze (`Assets/Shaders/fragment.fs`) mo�na zmieni� sk�adow� zwierciadlan� z Phonga na Blinna, korzystaj�c z uniforma `blinn`.
W aplikacji mo�na zmienia� warto�� zmiennej `blinn` za pomoc� klawisza `B`.
[�r�d�o](https://learnopengl.com/Advanced-Lighting/Advanced-Lighting)
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