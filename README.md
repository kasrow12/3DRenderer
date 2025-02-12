# 3DRenderer
## Instalacja
```
git clone --recursive https://github.com/kasrow12/3DRenderer.git
cd 3DRenderer
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release && build\Release\My3DRenderer.exe
```

## Flaga na wietrze (p³at Beziera) (Tessellation Shader)
Dziêki zastosowaniu shaderów teselacji, mo¿liwe jest wygenerowanie dodatkowych wierzcho³ków, które pozwalaj¹ na zwiêkszenie rozdzielczoœci powierzchni.

Do teselacji potrzebne nam s¹:
- [Control Shader](Assets/Shaders/tessControl.tcs) - kontroluje iloœæ generowanych wierzcho³ków, parametr tessLevel okreœla iloœæ generowanych dodatkowych wierzcho³ków na krawêdŸ
- [Evaluation Shader](Assets/Shaders/tessEval.tes) - oblicza pozycje nowych wierzcho³ków i wylicza wartoœci powierzchni Beziera

Potok renderowania z teselacj¹ wygl¹da nastêpuj¹co:
1. Vertex Shader
2. Tessellation Control Shader
3. Tessellation Evaluation Shader
4. Fragment Shader

W aplikacji u¿ytkownik mo¿e zmieniæ rozdzielczoœæ powierzchni Beziera w menu. Aby lepiej uwidoczniæ efekt, mo¿na w³¹czyæ tryb wyœwietlania siatki (klawisz `F`).

Aby w³¹czyæ teselacjê nale¿a³o wywo³aæ funkcjê 
```glPatchParameteri(GL_PATCH_VERTICES, 16);```
, dziêki czemu OpenGL wie, ¿e p³at wejœciowy bêdzie siê sk³adaæ z 16 wierzcho³ków.

Nastêpnie skoro mamy tylko jeden p³at, to wyœwietlenie jego mo¿na zrealizowaæ poprzez
```glDrawArrays(GL_PATCHES, 0, 16);```.


## Zmiana sk³adowej zwierciadlanej Phong/Blinn
We [fragment shaderze](Assets/Shaders/fragment.fs) u¿yta sk³adowa zwierciadlana (Phonga/Blinn) zale¿y od uniformu `blinn`.
W aplikacji mo¿na zmieniaæ wartoœæ tej zmiennej, tym samym sk³adowej zwierciadlanej za pomoc¹ klawisza `B`.
[ród³o](https://learnopengl.com/Advanced-Lighting/Advanced-Lighting)
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