Helios
=========
CPU renderer I work on in my spare time to improve my understanding of rendering techniques and algorithms.

Usage:
-------------
```
Usage of ./Helios :
-i path/filename
         Specify scene input file (Current supported scene formats: glTF 2.0)
-o path/filename
         Specify output filename (Current supported formats: PNG)
--width value
         Specify image width in pixels (default value 1920)
--height value
         Specify image height in pixels (default value 1080)
```
Roadmap
------------
### Scene formats
- [x] glTF 2.0
- [ ] FBX
- [ ] COLLADA
### Integrator
- [x] Ray tracing
- [ ] Path tracing
- [ ] Bidirectional path tracing
### Lights
- [x] Directional light
- [x] Point light
- [ ] Spot light
- [ ] Area light
- [ ] Emitters
### BxDFs
- [x] Lambertian BRDF
- [ ] Mirror BRDF
- [ ] Refraction BTDF
- [ ] Oren-Nayar BRDF
- [ ] Cook-Torrance BRDF
### Bump mapping
- [ ] Bump mapping
- [x] Normal mapping
- [ ] Displacement mapping
### Post-process
- [ ] Tonemapping
- [ ] Denoise?
### Antialiasing
- [ ] Supersampling
### Plugin
- [ ] Blender plugin



![](https://github.com/SergeiBorzov/Helios/blob/master/test_scenes/box/result.png)
![](https://github.com/SergeiBorzov/Helios/blob/master/test_scenes/bunny/result.png)
