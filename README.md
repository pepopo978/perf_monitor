Tool to monitor performance stats in 30 second intervals for core game functions as well as addons.

From the timings I've observed it seems like the client game logic each frame is kicked off by CSimpleTop(UIParent I think) OnRender and OnUpdate functions.  It's really hard to follow the exact call flow with how abstracted everything is so this could be wrong, but my `[Total] Render` is based on UIParent render.  If you're wondering why total render time is a lot less than the 30 second measuring window I'm not entirely sure, some of that time is spent idling I assuming waiting for networking updates or gpu renders to finish.

If any of these assumptions I've made are wrong or you find better ways to measure things please let me know or make a pull request :)

