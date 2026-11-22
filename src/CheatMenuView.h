// CheatMenuView.h — floating flame-logo overlay + cheat panel.
//
// Layout / colours / Vietnamese wording follow mockup/cheat-menu-mockup.html
// (the in-game scene behind it is NOT reproduced — this view is only the
// floating logo + panel). The logo is draggable, the panel opens below the
// logo when there is room and above it otherwise, and everything stays
// inside the window. The view does no per-frame work: the UI is only
// rebuilt when the user interacts, except the FPS badge, which uses a
// CADisplayLink that exists only while feature #40 is ON and updates its
// label once per second.
#import <UIKit/UIKit.h>

@interface CheatMenuView : UIView
// Attach to a window. Safe to call repeatedly; re-parents if needed.
- (void)attachToWindow:(UIWindow *)window;
@end
