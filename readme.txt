An explanation on how to get stuff to actually work:

Without kdTrees or antialiasing checked:
Multithreading is implemented with normal raytracing so you can render all the scenes, it's just slower than it would be with kdtree. Works normally.

With antialiasing checked:
Works, but is not multithreaded, so will display the original image before antialiasing it. It's also really slow and I'm not sure why, so for the larger trimeshes it's hard to see unless you wait a really long time. You can see that it works with some of the smaller ones with a smaller wait. I left the "antialiasing" statement in so that you can tell that it's actually antialiasing even though the un-aa'd image is displayed.

With kdTrees checked:
Creating the kdTree occurs on loading the file, so it hangs in the file select screen for a while. Additionally, if you want to remake the kdtree with a different node size, you need to go back into file select and select it again so it remakes the tree.

I used one late day for this!
