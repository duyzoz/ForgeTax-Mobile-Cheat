# HANDOFF — ForgeTax iOS 3.16.1 Cheat (dylib runtime injection)

Cây repo sẵn sàng push: `~/workspace/forgetax-ios/cheat-build/` (18 file + 2 báo cáo).
Trạng thái: **code xong, verify xong (CIPHER + AEGIS PASS_WITH_PROOF), 5 fix sau AEGIS đã áp dụng.**
**Chưa làm:** push GitHub, chạy workflow CI, test trên iPhone thật — đó là việc của main agent + Ethan.

## 1. Đã verify cái gì (bằng chứng)
- **Patch table — 58/58 khớp, 3 chiều độc lập** (CIPHER rồi AEGIS kiểm lại): `patch_table_verified.json` ↔ bytes gốc trong UnityFramework thật ↔ `src/patch_table.h` (regen byte-identical). 20 patch áp dụng được (14 MATCH + 6 DERIVED) + 37 call CHECKED + 1 MISMATCH. **39/39 tính năng #2–#40 đủ.**
  - IPA gốc SHA-256 `65f89d34…af87cdf` (149,673,760 B), UnityFramework SHA-256 `40a7f740…17f158` (119,049,072 B), cryptid=0, RVA == file offset (xác nhận từ header `__TEXT vmaddr=0`).
  - `scripts/verify_patch_table.py` (sau fix B) verify **58/58 entries** incl. call prologue + regen-header diff + stale-RVA guard; có kiểm chứng âm tính (sửa 1 byte → FAIL, exit 1).
- **Dry-run đóng gói — PASS** (AEGIS, dylib giả 0 byte, LIEF 1.0.0): LC_LOAD_DYLIB `@executable_path/Frameworks/ForgeTaxCheat.dylib` có mặt (48 vs 47 commands), UnityFramework SHA không đổi, Info.plist không đổi, dylib đúng `Frameworks/`, repack 147,823,552 B `unzip -t` OK. IPA gốc không bị sửa.
- **Rà soát code — PASS:** không string IAP trong code/scripts/workflow/JSON (chỉ còn nhắc bằng lời trong 2 file báo cáo .md — xem §4); không sửa save/asset; không hook render loop; không network call. Timer duy nhất 1.0s, tối đa 60 lần, tự dừng khi attach; CADisplayLink chỉ khi FPS bật, label update 1 Hz. PatchEngine verify original bytes trước khi ghi. Mọi call game null-check + try/catch. AEGIS đối chiếu `dump.cs`: 14 symbol IL2CPP đều exported, chuỗi instance pause→page→tools→room→campaign, field offset, enum `ForgeOutcome` (1/2/3/4), mọi RVA đều khớp.
- **Mockup vs README:** mockup có 38 mục F (id 2–39) + FPS id 40 = 39 tính năng, khớp README. Nhãn mockup hard-code "38 tính năng" sai 1 (menu thật đếm động 37 mục CHEAT ENGINE vì #32 thuộc SETTING) — không phải lỗi code.

## 2. Hai lệch quan trọng đã xử lý (không còn provisional)
- **#4 Guard — patch ĐỔI hoàn toàn.** Patch cũ `MOV w0,#3; RET` tại entry 0x2751550 bị LOẠI (hạ Great(4) → Success(3), trái README "Great vẫn Great"). Patch mới DERIVED tại **0x2751574**: `csel w0,w8,w9,eq` → `mov w0,w8` (`e003082a`) — luôn AlwaysSuccess nhưng giữ Great. **Tác dụng phụ (đã ghi trong code + README + HANDOFF-SENSOR):** khi patch #4 BẬT, CycleForgeMode ("Đổi mode") không còn tác dụng lên Guard; tắt patch thì đổi mode có tác dụng lại. #4 giờ là toggle patch đơn + nút phụ "Đổi mode".
- **#14 OfferPrice — chỉ dùng 0x275D430.** Địa chỉ cũ 0x275D428 là SAI (đầu hàm, `ldr w8,[x0,#0x200]`); entry MISMATCH `do_not_apply=true`, không có trong mảng áp dụng (chỉ còn trong comment + guard của verify script).

## 3. 5 fix sau AEGIS (đã áp dụng, có kiểm chứng)
- **A.** `scripts/inject_dylib.py` — check idempotency chấp nhận cả `LOAD_DYLIB`/`LC_LOAD_DYLIB` (LIEF 1.0.0 trả `LOAD_DYLIB`); inject 2 lần liên tiếp chỉ còn 1 entry.
- **B.** `scripts/verify_patch_table.py` — implement thật check regen-header + stale-RVA guard; verify cả 58 entries (trước chỉ 20).
- **C.** `src/PatchEngine.mm` — bỏ `"PROVISIONAL"` khỏi allowlist apply (chỉ MATCH/DERIVED/CHECKED).
- **D.** `src/GameBridge.mm` — **bỏ hẳn cache** `s_toolsCache`/`s_campaignCache`; resolve fresh mỗi lần gọi (user bấm nút, không per-frame) để không dùng pointer stale khi game destroy/recreate object.
- **E.** Workflow thêm bước assert LC_LOAD_DYLIB có trong IPA cuối (otool, fail nếu thiếu).

## 4. Cái gì CHƯA verify được (chỉ CI / Ethan làm được)
- **Biên dịch thật:** không có macOS/Xcode ở đây. Code chỉ được kiểm tra tĩnh (brace/paren, @selector có method, scripts compile, YAML parse). Lỗi cú pháp ObjC++ (nếu có) chỉ CI mới bắt được.
- **Chạy thật trên iPhone 7 Plus:** launch, logo hiện ngay, resolve instance dev-tool, patch có ăn thật không, FPS có tụt không. Các điểm SENSOR đã disclose: instance dev page có thể null tới khi mở pause menu (menu báo "chưa gọi được", không fake); fallback dựng `ForgeDeveloperTools(room)` chưa kiểm chứng live; layout `Nullable<QualityTier>` (#32), chuỗi campaign cho #12, `vm_protect` patch trên TrollStore, `dlsym` symbol IL2CPP — đều cần live test.
- **Release asset base IPA** trên duyzoz/ForgeTax-Mobile-Cheat đã tồn tại chưa — workflow sẽ fail rõ ràng nếu chưa có (không tải từ nguồn khác).
- **Semantics runtime sâu** của từng patch: mới verify tới mức bytes + capstone + `dump.cs`.
- **dylib thật qua ldid/TrollStore:** dry-run dùng file rỗng theo brief; bước `otool` trong workflow (mục E) cũng chỉ chạy trên macOS runner.
- Chuỗi mã sản phẩm IAP trong 2 file báo cáo đã được main agent gỡ bỏ trước khi push (chỉ còn câu văn nói về lệnh cấm, không còn chuỗi mã thật trong repo).

## 5. Các bước tiếp theo cho main agent
1. **Push toàn bộ `cheat-build/` lên duyzoz/ForgeTax-Mobile-Cheat, THAY TOÀN BỘ repo cũ** (repo hiện chứa 224 file remake cũ — theo lệnh Ethan; việc push là của main agent, không phải của job này).
2. **Upload base IPA gốc** làm release asset `base-ipa-3.16.1/com.soucean.fruityzoo_v3.16.1_und3fined.ipa` (file tại `~/workspace/browser_downloads/sess-124068829/browser-download-20261122T111630.179864570Z-0-com.soucean.fruityzoo_v3.16.1_und3fined.ipa`, SHA-256 `65f89d34…af87cdf`) — workflow cần nó mới chạy được bước (2).
3. Chạy workflow `build-cheat-ipa.yml` trên huyzoz fork (Actions chỉ chạy được ở huyzoz — theo quy tắc đã biết; xem workflow có chạy trên duyzoz không, nếu không thì dispatch qua huyzoz rồi chuyển artifact về).
4. Kiểm tra artifact `ForgeTax-Cheat-3.16.1.ipa` + SHA-256; đưa Ethan cài qua TrollStore trên iPhone 7 Plus, test theo checklist: logo hiện ngay khi vào game → bật từng tính năng → báo lỗi/crash/FPS.
5. **Không hứa "không crash tuyệt đối" với Ethan** — README đã ghi đúng mức (cảnh báo test tài khoản phụ trước).

## 6. Cấu trúc repo (18 file)
- `src/`: `ForgeTaxCheat.mm`, `CheatMenuView.h/.mm`, `PatchEngine.h/.mm`, `GameBridge.h/.mm`, `FeatureCatalog.h`, `patch_table.h`
- `scripts/`: `patch_table_to_header.py`, `verify_patch_table.py`, `inject_dylib.py`
- `.github/workflows/build-cheat-ipa.yml`, `README.md` (VI), `LICENSE` (MIT), `.gitignore`, `patch_table_verified.json`
- Báo cáo: `HANDOFF-SENSOR.md` (ghi chú SENSOR + mục FIXES sau AEGIS), `AEGIS-REPORT.md` (báo cáo verify đầy đủ)
- **Không có trong repo (đúng như thiết kế):** IPA gốc, UnityFramework, dylib đã biên dịch, 2 ô IAP / no-ads / TestMode / capacity-to-11 (bị loại theo README §4 + lệnh Ethan).
