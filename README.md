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
- [Control Shader](Assets/Shaders/tessControl.tcs) - kontroluje ilo�� generowanych wierzcho�k�w, parametr tessLevel okre�la ilo�� generowanych dodatkowych wierzcho�k�w na kraw�d�
- [Evaluation Shader](Assets/Shaders/tessEval.tes) - oblicza pozycje nowych wierzcho�k�w i wylicza warto�ci powierzchni Beziera

Potok renderowania z teselacj� wygl�da nast�puj�co:
1. Vertex Shader
2. Tessellation Control Shader
3. Tessellation Evaluation Shader
4. Fragment Shader

W aplikacji u�ytkownik mo�e zmieni� rozdzielczo�� powierzchni Beziera w menu. Aby lepiej uwidoczni� efekt, mo�na w��czy� tryb wy�wietlania siatki (klawisz `F`).

Aby w��czy� teselacj� nale�a�o wywo�a� funkcj� 
```glPatchParameteri(GL_PATCH_VERTICES, 16);```
, dzi�ki czemu OpenGL wie, �e p�at wej�ciowy b�dzie si� sk�ada� z 16 wierzcho�k�w.

Nast�pnie skoro mamy tylko jeden p�at, to wy�wietlenie jego mo�na zrealizowa� poprzez
```glDrawArrays(GL_PATCHES, 0, 16);```.


## Zmiana sk�adowej zwierciadlanej Phong/Blinn
We [fragment shaderze](Assets/Shaders/fragment.fs) u�yta sk�adowa zwierciadlana (Phonga/Blinn) zale�y od uniformu `blinn`.
W aplikacji mo�na zmienia� warto�� tej zmiennej, tym samym sk�adowej zwierciadlanej za pomoc� klawisza `B`.
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