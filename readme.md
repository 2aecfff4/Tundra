# Tundra 

![Screenshot](/docs/screenshot.png?raw=true "Screenshot")

Tundra is an experimental project that aims to explore C++ 20 and modern GPU-driven rendering techniques.

I've made a blog post about implementation details. You can read it [here](https://2aecfff4.github.io/posts/tundra-behing-the-scenes/).

# Build
Keep in mind that this code is experimental and requires a number of bugs to be fixed before it becomes usable. Most likely it won't happen as I have accomplished my goals.

To build, only CMake, DXC and Python are needed. 

Shaders have to be build manually in the `/shaders` directory and copied to the `/assets/` folder where the binary is located.

    
