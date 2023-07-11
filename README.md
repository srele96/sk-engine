# sk-engine

Engine for skill demonstration purposes assembled without tutorials.

## How to build

For build instructions see [`vcpkg.json`](./vcpkg.json) because it's package manager so I figured building documentation should be there.

## Creation process

Babylon documentation - [First Scene and Model](https://doc.babylonjs.com/features/introductionToFeatures/chap1/first_scene)

Whether you are creating a whole world or just placing one model into a web page you need a scene to contain the world or model, a camera to view it, a light to illuminate it and, of course, at least one viewable object as a model. All models, whether just a box or a complex character, are made from a mesh of triangles or facets.

- Scene
- Camera
- Light
- Object

### Reading [Babylon.js](https://github.com/BabylonJS/Babylon.js/tree/master) implementation

To understand engine concepts, I read implementation of `Babylon.js`'s `scene`, `engine`, `simpleEngine`, `abstractScene`, `material`, `mesh`, `shaders`.

Locations:

- [`abstractScene`](https://github.com/BabylonJS/Babylon.js/blob/master/packages/dev/core/src/abstractScene.ts#L38)
- [`engine`](https://github.com/BabylonJS/Babylon.js/blob/master/packages/dev/core/src/Engines/engine.ts)
- [`standardMaterial`](https://github.com/BabylonJS/Babylon.js/blob/master/packages/dev/core/src/Materials/standardMaterial.ts)
- [`scene`](https://github.com/BabylonJS/Babylon.js/blob/master/packages/dev/core/src/scene.ts#L4516)
- [Smallest possible API](https://github.com/BabylonJSGuide/JSProject/blob/main/index.html)
- [Documentation to see the class relationships](https://doc.babylonjs.com/typedoc/classes/BABYLON.Engine#constructor)

### Conceptual understanding

I talked with ChatGPT to create conceptual map of how to implement the rendering API. After roughly 16 hours and an attempt to design public api using structure below, I took a walk because I felt burnout. While walking I thought about different things and how to complete renderer in time.

I realized my weakness was that I don't have great grasp of how to use `OpenGL` API so I decided to put together code with help of ChatGPT starting from concepts going to the code to gain more complete understanding. A solution was to create something fairly complex: A movable camera in 3D world with help of `glm` because I have yet to hone mathematical skills to do the math myself. 3D objects in the world that I can observe as I move the camera using mouse and keyboard controls.

That would yield me some mental images of how it fits and works together, how to use `OpenGL` API and give me ideas of most frequently called commands and how to assemble my own API that would allow me to add meshes to the scene in somewhat user-friendly way.

With those fundamentals I could approach conceptual design again and hopefully make it work. I disliked the feeling that I don't understand every single line of code because I had fear to change stuff because I didn't know if it will break something. However, I could make all of that work together, so I guess my conceptual understanding was solid because I knew what logic I need and where to place it. Obviously, code wouldn't work if I didn't know how it works... duh!

[See pictures of the process.](./picture/)

### It is a lot...

After digging through the `Babylon.js` engine and attempts to figure out all the math required to setup camera, I feel like it is a lot... In their engine I saw that they have at least three cameras and they involve matrix computations. Matrices on their own are OK, while I am not that great with trigonometry because I need practice. I learned how much math the engine requires because all data that goes through the engine relies on mathematical computations. Not even the `glm` library helps with math if I don't have in my mind how it fits together. Once one matrix fails and doesn't do what I expect it to, that's it, I don't know which value is messing it up.

OpenGL and learning that on it's own is fine, however there is a lot math going on which I should focus and learn to effectively work with computer graphics. Currently it seems primary targets are linear algebra and trigonometry, however I believe there are other areas of math that play crucial role. Perhaps algorithms for object surfaces.

### What can I currently make?

I am confident I could create a `Node` class that contains object data and takes in a texture or whatever which is just an OpenGL program which accepts uniforms and I could play with different matrices and objects in the scene that way... I could have some kind of API doing that! The current code that creates a window and renders data to it could be a separate thing for itself as a documentation of the process so far... I had so much fun doing all of that! I feel like I could just keep going, and damn I really need to learn the math.

### Maybe I overwhelmed myself?

I've read implementation of various methods and classes of `Babylon.js` to gain understanding in some common patterns that widely adopted engine uses. I've read so much and I feel like I have so much information with very limited experience so far. They use advanced methods that I am not familiar with, however I've gained quite a few ideas on what to do now. I could play with frustum, matrices, rotations, etc... Figure out math by pen, paper, tutorials, books, etc... then implement that math using OpenGL and programming!

Let's let it sink in!
