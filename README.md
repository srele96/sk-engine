# sk-engine

Engine for skill demonstration purposes assembled without tutorials.

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
