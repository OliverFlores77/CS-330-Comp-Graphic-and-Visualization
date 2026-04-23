### CS 330 README by Oliver Flores

### How do I approach designing software?

I like to break things down into simple shapes first. For this scene, I picked a pencil, notebook, tissue box, and coffee mug because they are things I see every day and they are made of basic shapes like cylinders and boxes. That kept it from getting too overwhelming. I also tried to think about how someone would actually explore the scene, so I made the camera controls feel like a video game with WASD and mouse look.

**What new design skills has your work on the project helped you to craft?**
I figured out how to put multiple shapes together into one object. The pencil was a cylinder for the body, cones for the tips, and another cylinder for the eraser. Getting everything to line up took a lot of trial and error. I also learned a lot about lighting. I used warm yellow lights in the front and back and cool white lights on the sides to get rid of dark spots.

**What design process did you follow for your project work?**
I built things one at a time. First I made the tabletop with the marble texture, then added the pencil, notebook, and so on. After each object I messed with the lighting and camera so I could see if it looked right. The coffee mug handle was the hardest because I did not even know what a torus was before this class. I had to play around with scaling and position until it finally worked.

**How could tactics from your design approach be applied in future work?**
The whole start with basic shapes, then texture, then light approach would work for pretty much any 3D scene. And because I put everything into functions like DefineObjectMaterials and SetupSceneLights, someone else could take my code and just swap out textures or move the lights around without having to rewrite a bunch of stuff.

### How do I approach developing programs?

I write a little piece at a time and make sure it works before moving on. For this project, I got one textured cylinder to show up correctly, then added rotations. I rotated everything 90 degrees on Z to make it flat, then 45 on Y so it was not straight. After that worked, I just repeated the pattern for the other objects.

**What new development strategies did you use while working on your 3D scene?**
I learned to put texture loading and material stuff into separate functions so the code did not get too messy. I also used a copy and modify strategy. When I added the coffee mug, I just copied the pattern from the pencil and changed the shape and position.

**How did iteration factor into your development?**
A lot. I kept tweaking numbers until things looked right. For the materials, I messed with ambient strength and ended up at 1.0, and shininess at 2.0. I just guessed and checked. I moved the lights around a bunch of times to get rid of shadows. The pencil tip alone took several tries. But each change was small, so I never totally broke the whole scene.

**How has your approach to developing code evolved throughout the milestones, which led you to the project's completion?**
Early on I just wrote everything in one big block. By the end I was automatically making functions for things I repeated. I also started using the different camera views to debug. Pressing O for orthographic mode helped me see if objects were actually lined up correctly, and pressing P switched back to the normal perspective view.

### How can computer science help me in reaching my goals?

Computer science lets me take ideas and make something interactive. This project turned a bunch of math like vectors, rotations, and matrices into a scene I could walk around in. Whether I end up making games, simulations, or data visualizations, knowing how to build a 3D world and move a camera through it is super useful.

**How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?**
Graphics makes you learn coordinate systems and linear algebra by actually doing it. That stuff shows up in robotics, data plots, and even app animations. Before this class I had heard words like torus and ambient strength but did not really get them. Now I do.

**How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future professional pathway?**
A lot of jobs use 3D visualization. Architecture has walkthroughs, medical imaging has 3D reconstructions, and manufacturing has digital twins. Even game engines like Unity and Unreal work the same way under the hood. My experience writing OpenGL code means I understand what those engines are actually doing. Plus, debugging camera controls and object rotations is just good practice for problem solving in any programming job.
