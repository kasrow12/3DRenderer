# 3DRenderer
Prosty renderer 3D napisany w C++ z wykorzystaniem OpenGL.

## Technologie i Biblioteki
- C++
- OpenGL
- GLAD
- GLSL
- GLFW
- GLM
- ImGui (Dear ImGui)
- stb_image
- CMake

## Główne Funkcjonalności
- **Ładowanie Modeli 3D:** Obsługa ładowania modeli z plików (np. format `.obj` dla obiektów takich jak pociąg, dinozaury, podłoże).
- **Różne tryby kamery:** statyczna obserwująca scenę, statyczna śledząca obiekt, przyczepiona do obiektu (np. pociągu), swobodna (free-look).
- **Oświetlenie:**
    - Model oświetlenia Phong oraz Blinn-Phong (dynamicznie przełączane).
    - Światła punktowe
    - Światło kierunkowe
    - Reflektor (Spotlight) - światło czołowe pociągu
- **Efekt mgły**
- **GUI**
- **Powierzchnie Beziera:**
    - Renderowanie płata Beziera z wykorzystaniem shaderów teselacji, z możliwością dynamicznej zmiany poziomu teselacji.

## Instalacja
```
git clone --recursive https://github.com/kasrow12/3DRenderer.git
cd 3DRenderer
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release && build\Release\My3DRenderer.exe
```

## Zrzuty ekranu
![3d](https://github.com/user-attachments/assets/6231b504-6243-40f5-ba53-e7f88d172f75)
Widok siatki trójkątów
![wireframe](https://github.com/user-attachments/assets/37ea9c5e-d8f6-453a-be9a-226deb4b2aaf)
Efekt mgły
![fog](https://github.com/user-attachments/assets/cc41c48c-524d-420a-8727-8df494810f31)

## Sterowanie
| Klawisz      | Akcja                             |
|--------------|-----------------------------------|
| 1            | Statyczna kamera                  |
| 2            | Śledząca kamera                   |
| 3            | Kamera z perspektywy pociągu      |
| 4            | Kamera swobodna                   |
| F            | Przełącz widok siatki (Wireframe) |
| B            | Przełącz Blinn-Phong              |
| N            | Przełącz Dzień/Noc                |
| WASD         | Poruszanie kamerą                 |
| Space        | W górę                            |
| Left Shift   | W dół                             |
| Left Control | Przechwycenie myszy               |
| Scroll       | Zoom                              |

## Flaga na wietrze (płat Beziera) (Tessellation Shader)
Dzięki zastosowaniu shaderów teselacji, możliwe jest wygenerowanie dodatkowych wierzchołków, które pozwalają na zwiększenie rozdzielczości powierzchni.

Do teselacji potrzebne nam są:
- [Control Shader](Assets/Shaders/tessControl.tcs) - kontroluje ilość generowanych wierzchołków, parametr tessLevel określa ilość generowanych dodatkowych wierzchołków na krawędź
- [Evaluation Shader](Assets/Shaders/tessEval.tes) - oblicza pozycje nowych wierzchołków i wylicza wartości powierzchni Beziera

Potok renderowania z teselacją wygląda następująco:
1. Vertex Shader
2. Tessellation Control Shader
3. Tessellation Evaluation Shader
4. Fragment Shader

W aplikacji użytkownik może zmienić rozdzielczość powierzchni Beziera w menu. Aby lepiej uwidocznić efekt, można włączyć tryb wyświetlania siatki (klawisz `F`).

Aby włączyć teselację należało wywołać funkcję 
```glPatchParameteri(GL_PATCH_VERTICES, 16);```
, dzięki czemu OpenGL wie, że płat wejściowy będzie się składać z 16 wierzchołków.

Następnie skoro mamy tylko jeden płat, to wyświetlenie jego można zrealizować poprzez
```glDrawArrays(GL_PATCHES, 0, 16);```.


## Zmiana składowej zwierciadlanej Phong/Blinn
We [fragment shaderze](Assets/Shaders/fragment.fs) użyta składowa zwierciadlana (Phonga/Blinn) zależy od uniformu `blinn`.
W aplikacji można zmieniać wartość tej zmiennej, tym samym składowej zwierciadlanej za pomocą klawisza `B`.
[Źródło](https://learnopengl.com/Advanced-Lighting/Advanced-Lighting)
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
