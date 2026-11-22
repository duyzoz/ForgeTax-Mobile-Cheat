// ForgeTaxCheat.mm — dylib entry point.
//
// The constructor runs at launch and does almost nothing: it schedules the
// overlay attach on the main queue. Attaching watches for the game's key
// window (UIWindowDidBecomeKeyNotification) plus a light 1-second retry
// timer that stops itself after 60 tries or as soon as the overlay is
// attached. After attach there are NO background timers and NO per-frame
// work — the only recurring work in the whole dylib is the FPS
// CADisplayLink, which exists only while feature #40 is ON.
//
// Nothing here touches the game binary on disk, writes any file, or calls
// any game function at launch. Every feature defaults OFF.
#import <UIKit/UIKit.h>
#import "CheatMenuView.h"

static CheatMenuView *s_menuView = nil;
static NSTimer *s_attachTimer = nil;
static NSInteger s_attachTries = 0;

static UIWindow *FTC_KeyWindow(void) {
    @try {
        UIApplication *app = [UIApplication sharedApplication];
        if (!app) return nil;
        UIWindow *key = app.keyWindow;
        if (key) return key;
        // iOS 13+ multi-scene fallback: first visible window of any scene.
        for (UIScene *scene in app.connectedScenes) {
            if (![scene isKindOfClass:[UIWindowScene class]]) continue;
            for (UIWindow *w in ((UIWindowScene *)scene).windows) {
                if (w.isKeyWindow) return w;
            }
        }
        for (UIScene *scene in app.connectedScenes) {
            if (![scene isKindOfClass:[UIWindowScene class]]) continue;
            for (UIWindow *w in ((UIWindowScene *)scene).windows) {
                if (!w.hidden) return w;
            }
        }
    } @catch (...) {}
    return nil;
}

static void FTC_TryAttach(void) {
    @try {
        UIWindow *window = FTC_KeyWindow();
        if (!window) return;
        if (!s_menuView) {
            s_menuView = [[CheatMenuView alloc] initWithFrame:window.bounds];
        }
        [s_menuView attachToWindow:window];
        [s_attachTimer invalidate];
        s_attachTimer = nil;
    } @catch (...) {}
}

static void FTC_OnWindowDidBecomeKey(NSNotification *note) {
    @try {
        UIWindow *window = (UIWindow *)note.object;
        if (![window isKindOfClass:[UIWindow class]]) {
            window = FTC_KeyWindow();
        }
        if (!window) return;
        if (!s_menuView) {
            s_menuView = [[CheatMenuView alloc] initWithFrame:window.bounds];
        }
        [s_menuView attachToWindow:window];
        [s_attachTimer invalidate];
        s_attachTimer = nil;
    } @catch (...) {}
}

__attribute__((constructor))
static void ForgeTaxCheatInit(void) {
    @autoreleasepool {
        dispatch_async(dispatch_get_main_queue(), ^{
            @try {
                [[NSNotificationCenter defaultCenter]
                    addObserverForName:UIWindowDidBecomeKeyNotification
                                object:nil
                                 queue:[NSOperationQueue mainQueue]
                            usingBlock:^(NSNotification *note) { FTC_OnWindowDidBecomeKey(note); }];
                FTC_TryAttach();
                if (!s_menuView || !s_menuView.superview) {
                    // Light retry: 1s interval, stops on attach or after
                    // 60 tries. No timer survives past attach.
                    s_attachTries = 0;
                    s_attachTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                                     repeats:YES
                                                                       block:^(NSTimer *t) {
                        s_attachTries++;
                        FTC_TryAttach();
                        if ((!s_menuView || !s_menuView.superview) && s_attachTries >= 60) {
                            [t invalidate];
                            s_attachTimer = nil;
                        }
                    }];
                }
            } @catch (...) {}
        });
    }
}
