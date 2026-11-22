# HANDOFF — SENSOR (BUILD CHEAT FORGETAX iOS)

Ngày: 2026-11-22. Repo: `~/workspace/forgetax-ios/cheat-build/`.
Trạng thái: code xong, **KHÔNG push, KHÔNG chạy workflow** (theo brief).
Chưa biên dịch được ở đây (không có macOS/Xcode) — CI mới biên dịch được.

## 1. patch_table.h — ĐÃ VERIFIED (không còn provisional)

- Nguồn duy nhất: `patch_table_verified.json` (CIPHER, capstone 5.0.7).
- 58 entries: **20 patch áp dụng được** (14 MATCH + 6 DERIVED) + 37 call
  target CHECKED (chỉ tham chiếu, không sinh bytes) + **1 MISMATCH**.
- Sinh bằng: `python3 scripts/patch_table_to_header.py
  patch_table_verified.json src/patch_table.h` (đã kiểm tra header đồng bộ).
- UnityFramework SHA-256:
  `40a7f74082350afc2312b4c6e9fc224258e8900b01051825393936714c17f158`
  (119.049.072 byte, cryptid=0, RVA == file offset — đã xác nhận lại từ
  header Mach-O trong JSON meta và khớp file local).
- IPA gốc SHA-256:
  `65f89d3496820087b9476c826c0692ecca8b61c45dbe8a05a78d42f44af87cdf`.
- Kiểm chứng lại: `python3 scripts/verify_patch_table.py
  patch_table_verified.json --unity <UnityFramework>` → **VERIFY OK**
  (20/20 original bytes khớp file local).
- Bytes provisional từ README đã bỏ hết. Bảng provisional cũ chỉ còn trong
  `/tmp/provisional_patch_table.json` (không nằm trong repo).

### #4 Guard — PATCH ĐỔI HOÀN TOÀN (theo yêu cầu CIPHER)
- Patch cũ `MOV w0,#3; RET` tại entry `0x2751550` **đã loại** (hạ Great(4)
  xuống Success(3), trái README "Great vẫn Great"). Chuỗi bytes cũ chỉ còn
  xuất hiện trong **comment giải thích lý do loại** ở `patch_table.h:88`
  (trích từ notes của JSON) — không có trong bất kỳ mảng bytes áp dụng nào.
- Patch mới: `0x2751574`, `csel w0,w8,w9,eq` (`00 01 89 1a`) →
  `mov w0,w8` (`e0 03 08 2a`), status DERIVED. Luôn trả kết quả
  AlwaysSuccess (3, hoặc 4 nếu input là Great).
- **Tác dụng phụ (ghi trong code comment + README + đây):** khi patch #4
  đang BẬT, CycleForgeMode (nút phụ "Đổi mode") không còn tác dụng lên
  Guard nữa — luôn AlwaysSuccess. Tắt patch #4 để đổi mode có tác dụng
  trở lại.
- Triển khai: #4 là toggle patch đơn "Rèn luôn thành công (giữ Great)";
  nút phụ "Đổi mode" gọi CycleForgeMode 1 lần và báo rõ trong status nếu
  patch đang bật.

### #14 OfferPrice — CHỈ dùng 0x275D430
- Entry MISMATCH `0x275D428` (`do_not_apply=true`, file bytes là
  `ldr w8,[x0,#0x200]` đầu hàm) **không có trong bảng C áp dụng** — chỉ
  xuất hiện trong comment "DISABLED entries" của header + ghi chú README.
- Entry áp dụng duy nhất: `0x275D430`, `1F 05 00 71` → `1F 01 08 6B`, MATCH.
- `scripts/verify_patch_table.py` có guard riêng: fail nếu `0x275D428`
  trở thành applyable.

### 6 patch DERIVED (lấy nguyên từ JSON, không tự suy lại)
#4 Guard 0x2751574; #5 FailureChance 0x2750A24 (`FMOV d0,xzr; RET`);
#5 TopGradeBreak 0x27739E8 (`MOV w0,#0; RET`); #19 get_PlayerInvulnerable
0x277639C; #20 get_Stunned 0x2777714; #21 get_Enraged 0x2777C8C.
Các patch MATCH còn lại lấy y nguyên original/patched bytes từ JSON.

## 2. Đối chiếu mockup vs README (số mục)
- Mockup `F` = **38 mục, id 2–39** + `FPS_ROW` id 40 riêng = **39 tính năng**,
  khớp README (39 tính năng #2–#40, #1 đã bỏ). Không thiếu tính năng nào.
- **Lệch duy nhất:** header nhóm CHEAT ENGINE trong mockup ghi
  "38 tính năng" nhưng thực tế render **37 mục** (38 trong `F` trừ #32
  thuộc nhóm SETTING). Nhóm SETTING ghi "2 mục" là đúng (#32 + #40).
  Menu thật đếm động từ `FeatureCatalog.h` nên hiển thị đúng 37.
- Các lệch UI có chủ ý so với mockup (mockup gán toggle cho mọi thứ,
  không phân biệt ngữ nghĩa thật):
  - One-shot (#2, #3, #8, #23, #24, #25, #26, #27, #29, #30, #31, #37)
    dùng nút **"Chạy"**, không phải switch — switch chỉ dành cho patch
    toggle và dev-toggle.
  - #4 từ seg 3 lựa chọn → toggle patch + nút phụ "Đổi mode" (xem §1).
  - #7: switch God mode + nút phụ "Giết ngay" (KillDragon).
  - #12: mockup stepper 0–99 → seg chọn loại vé (Damage/Hold/Success/
    Great) + nút "+1 vé" (AddRunTickets cần outcome + count).
  - #28: seg "-1/+1 ngày" — mỗi lần bấm là 1 lần gọi (SkipDay/NextDay),
    không giữ trạng thái mode.
  - #6 / #9: stepper + nút "Áp dụng"/"Nhảy" (chỉnh số không tự gọi hàm).
  - Logo lửa: path SVG lấy verbatim từ mockup; màu/token CSS giữ nguyên;
    font hệ thống San Francisco (mockup dùng Lilita One/Fredoka nhưng
    brief cấm bundle font lạ).

## 3. Cấu trúc dylib
- `src/ForgeTaxCheat.mm` — constructor: đăng ký
  `UIWindowDidBecomeKeyNotification` + timer retry 1s (tối đa 60 lần,
  tự dừng khi gắn). Sau khi gắn: không timer nền, không hook render loop.
- `src/CheatMenuView.h/.mm` — logo 56px (gradient cam, glow, flame SVG),
  kéo di chuyển (UIPanGestureRecognizer), panel kẹp trong màn hình, sổ
  xuống nếu đủ chỗ không thì sổ lên; header FORGE TAXX + 3 pill; 7 nhóm
  gập/mở + SETTING; toggle/segment/stepper; mô tả ẩn (bấm tên để xem);
  status từng dòng ("Đã chạy" / "Đã bật" / "chưa gọi được: ...").
  FPS badge: CADisplayLink chỉ tạo khi #40 bật, 1s cập nhật label 1 lần.
- `src/PatchEngine.h/.mm` — tìm UnityFramework qua dyld, `vm_protect`
  RW → **verify original bytes khớp mới ghi** → `memcpy` → protect lại
  R+X → `sys_icache_invalidate`. Không khớp → từ chối + báo lỗi trong menu.
- `src/GameBridge.h/.mm` — gọi hàm dev-tool qua `il2cpp_runtime_invoke`
  (dlsym các symbol `il2cpp_*`; đã kiểm tra chuỗi symbol có trong binary).
  Instance: `ForgePauseMenu` (MonoBehaviour, `ForgeTax.Menu`) →
  `developerPage` → `tools` → (`room` → `campaign` cho AddRunTickets).
  Fallback: `FindObjectOfType(ForgeRoom)` + dựng `ForgeDeveloperTools(room)`
  qua `.ctor` — **chưa kiểm chứng live**.
- `src/FeatureCatalog.h` — metadata 39 tính năng (loại, mô tả, cách làm).
- `src/patch_table.h` — bảng patch duy nhất (sinh từ JSON).

## 4. Phân loại tính năng (loại → cách chạy)
- **PATCH toggle (19 tính năng, 20 site):** #4, #5 (2 site), #10 (+ FillFever
  1 lần khi bật), #11, #13, #14, #15, #16, #17, #18, #19, #20, #21, #22,
  #34, #35, #36, #38, #39.
- **ONESHOT:** #2 AddGold(9.999e99), #3/#37 GiveAllRelics, #8, #23, #24
  (+ nút phụ OpenSwordPicker), #25, #26, #27 (hỏi xác nhận trước),
  #29, #30, #31.
- **DEV_TOGGLE (gọi hàm flip 1 lần, menu tự track trạng thái, mặc định
  OFF):** #7 ToggleGodMode, #33 ToggleMerchantShelfSlot (credit-shelf,
  không phải ô trả phí), #40 ToggleFps (+ CADisplayLink badge).
- **MODE/STEP:** #6 SetAnvilGrade (stepper 0–999), #9 JumpToDay (1–24),
  #12 AddRunTickets (chọn outcome + "+1 vé"), #28 SkipDay/NextDay,
  #32 SetQualityTier (Thấp=0/Vừa=1/Cao=2).

## 5. Tính năng hạ cấp / không chắc chắn + lý do
1. **Mọi lệnh gọi dev-tool đều "chưa gọi được" cho tới khi instance
   resolve được.** `ForgeDeveloperPage` có thể là null cho tới khi pause
   menu tạo dev page — menu báo rõ và gợi ý "mở menu tạm dừng trong game
   rồi bấm lại". Không fake kết quả: chỉ báo "Đã chạy" khi method trả
   true / không exception.
2. **Fallback dựng `ForgeDeveloperTools(room)` chưa kiểm chứng live**
   (dev page mới là owner dự kiến). Nếu fallback thất bại, tính năng
   one-shot/dev-toggle báo NO_INSTANCE.
3. **#32 SetQualityTier:** layout `Nullable<QualityTier>` dạng struct
   `{int32 value; bool has_value}` chưa kiểm chứng live.
4. **#12 Vé merchant:** cần chuỗi tools→room→campaign; chưa kiểm chứng live.
5. **#4 NoBreak:** không còn là mode riêng — chỉ còn qua CycleForgeMode
   (xoay mode, không đọc được mode hiện tại) và vô hiệu khi patch #4 bật.
6. **#10 Fever:** tắt patch chỉ gỡ patch Expire; Fever đã nạp không rút lại.
7. **#7 KillDragon / #29 / #30 / #8 / #27:** method trả false khi chưa đủ
   điều kiện trong game (VD giết rồng khi không có rồng) — menu báo
   "Game từ chối lệnh", không crash.
8. **PatchEngine `vm_protect` + `sys_icache_invalidate`:** chuẩn cho
   runtime patch trên iOS, nhưng chưa chạy thật trên TrollStore/iPhone 7 Plus.
9. **Symbol IL2CPP:** chỉ mới thấy chuỗi tên symbol trong binary; việc
   `dlsym` có export được không chỉ runtime trên máy mới trả lời.
10. **Không biên dịch được ở đây** (không macOS/Xcode): mọi lỗi cú pháp
    ObjC++ chỉ CI mới bắt được. Đã kiểm tra tĩnh: brace/paren cân bằng,
    mọi `@selector` đều có method tương ứng, scripts compile OK, YAML OK.

## 6. Những gì CI mới kiểm chứng được
- Biên dịch dylib arm64 iOS 15.0 (clang trực tiếp, không Xcode project).
- `patch_table.h` đồng bộ với JSON (workflow diff, fail nếu lệch).
- `verify_patch_table.py` chạy với base IPA thật (SHA + original bytes).
- Inject `LC_LOAD_DYLIB` bằng LIEF + assert SHA UnityFramework
  trước/sau giống nhau + đóng gói lại + `ldid -S` + upload artifact.
- **CI không kiểm chứng được:** app có launch không, logo có hiện không,
  instance IL2CPP có resolve không, patch có ăn không, FPS có tụt không.
  Tất cả cần test live trên iPhone 7 Plus, tài khoản phụ trước.

## 7. File đã viết
- `~/workspace/forgetax-ios/cheat-build/README.md`
- `~/workspace/forgetax-ios/cheat-build/LICENSE` (MIT)
- `~/workspace/forgetax-ios/cheat-build/.gitignore`
- `~/workspace/forgetax-ios/cheat-build/patch_table_verified.json` (CIPHER — đã có sẵn, không sửa)
- `~/workspace/forgetax-ios/cheat-build/src/patch_table.h` (sinh từ JSON verified)
- `~/workspace/forgetax-ios/cheat-build/src/ForgeTaxCheat.mm`
- `~/workspace/forgetax-ios/cheat-build/src/CheatMenuView.h`
- `~/workspace/forgetax-ios/cheat-build/src/CheatMenuView.mm`
- `~/workspace/forgetax-ios/cheat-build/src/PatchEngine.h`
- `~/workspace/forgetax-ios/cheat-build/src/PatchEngine.mm`
- `~/workspace/forgetax-ios/cheat-build/src/GameBridge.h`
- `~/workspace/forgetax-ios/cheat-build/src/GameBridge.mm`
- `~/workspace/forgetax-ios/cheat-build/src/FeatureCatalog.h`
- `~/workspace/forgetax-ios/cheat-build/scripts/patch_table_to_header.py`
- `~/workspace/forgetax-ios/cheat-build/scripts/verify_patch_table.py`
- `~/workspace/forgetax-ios/cheat-build/scripts/inject_dylib.py`
- `~/workspace/forgetax-ios/cheat-build/.github/workflows/build-cheat-ipa.yml`
- `~/workspace/forgetax-ios/cheat-build/HANDOFF-SENSOR.md` (file này)

Không push GitHub, không chạy workflow, không liên hệ ai (theo brief).
Chuỗi mã sản phẩm IAP không xuất hiện ở bất kỳ đâu trong repo (đã grep).

## FIXES sau AEGIS (SENSOR-fix, 2026-11-22 — sửa đúng 5 mục A–E, không đổi gì khác, không push)

**A. `scripts/inject_dylib.py:61–76` — check idempotency chết dưới LIEF 1.0.0.**
Chấp nhận cả hai tên enum `LC_LOAD_DYLIB` và `LOAD_DYLIB` khi so `cmd.command.name`
(LIEF 1.0.0 trả `LOAD_DYLIB`). Kiểm chứng: chạy inject 2 lần liên tiếp trên bản
copy của exe chính (IPA gốc không chạm tới) — lần 1 `Added LC_LOAD_DYLIB…`,
lần 2 `already present, skipping`, exit 0 cả hai; LIEF đếm đúng **1 entry**
`LOAD_DYLIB @executable_path/Frameworks/ForgeTaxCheat.dylib`, tổng 48 commands;
UnityFramework không đổi (`40a7f740…`).

**B. `scripts/verify_patch_table.py` — docstring + cover 58/58 + check header thật.**
(1) Đã implement check 3 thật thay vì chỉ sửa docstring: regen header từ JSON
bằng `scripts/patch_table_to_header.py` ra file tạm và diff byte với
`src/patch_table.h` (dòng 124–170), và grep stale RVA bằng cách parse đúng
trường RVA của từng entry trong mảng `kFTC_PatchTable[]` (comment không tính —
cần thiết vì 0x275D428 có xuất hiện hợp lệ trong comment của entry 0x275D430).
Docstring dòng 9–22 cập nhật đúng code mới. (2) Vòng lặp chính giờ verify
`original_bytes` của **tất cả 58 entries** (20 patch + 37 call + 1 MISMATCH;
call entry verify prologue, MISMATCH verify bytes file thật tại địa chỉ đó),
vẫn exit nonzero khi lệch. Kiểm chứng với UnityFramework thật từ IPA gốc:
`original bytes checked for 58/58 entries / All 58 entries' original bytes
match the binary. / Header … matches a fresh regen … / Stale RVA … not an
applyable entry … OK. / VERIFY OK`, exit 0. Kiểm chứng âm tính (JSON copy
trong /tmp, không chạm repo): sửa 1 byte `original_bytes` của entry #2 →
FAIL `file has ff8301d1, table says ffffffff`, exit 1; sửa header lệch 1 chữ →
FAIL `src/patch_table.h is out of sync`, exit 1.

**C. `src/PatchEngine.mm:58–64` — bỏ `"PROVISIONAL"` khỏi `FTC_StatusApplyable`.**
Allowlist giờ chỉ MATCH/DERIVED/CHECKED; MISMATCH và mọi status lạ đều từ chối.
Comment trong hàm ghi rõ lý do. Kiểm chứng: `grep -rn PROVISIONAL src/` chỉ
còn trúng comment giải thích trong chính hàm này (không còn `strcmp` nào với
`PROVISIONAL`); không biên dịch được ở đây (không macOS/Xcode) — CI mới bắt
lỗi cú pháp ObjC++.

**D. `src/GameBridge.mm:145–153,177,218–230` — bỏ hẳn `s_toolsCache`/`s_campaignCache`.**
`ResolveTools()`/`ResolveCampaign()` resolve fresh mỗi lần gọi, không lưu
pointer nào qua các lần gọi; ghi chú "NO CACHING" trong code nêu rõ lý do
(scene reload / destroy-recreate → pointer cache thành dangling → crash thay
vì NO_INSTANCE) và rằng các call này chỉ chạy khi user bấm nút, không per-frame
nên chi phí không đáng kể. Mọi đường gọi (`InvokeTools*`, `FTC_SetQualityTier`,
`FTC_AddRunTickets`, `FTC_ToolsAvailable`) đều đi qua `ResolveTools()`/
`ResolveCampaign()` fresh. Kiểm chứng: `grep s_toolsCache|s_campaignCache
src/` chỉ còn trúng comment giải thích trong GameBridge.mm (không còn biến);
không còn `static Il2CppObject *` nào giữ instance game (chỉ còn `s_api` là
bảng function-pointer IL2CPP, load một lần có chủ ý).

**E. `.github/workflows/build-cheat-ipa.yml:106–124` — thêm bước assert LC_LOAD_DYLIB.**
Bước mới `Assert LC_LOAD_DYLIB present in final IPA`, đặt sau bước Fake-sign
(ldid re-pack) và trước Upload artifact, tức kiểm tra trên IPA cuối cùng:
giải nén IPA ra `/tmp/final-ipa-check`, `otool -l` exe chính, `grep -q
@executable_path/Frameworks/ForgeTaxCheat.dylib`, fail workflow nếu không thấy
(có in danh sách dylib để debug). Các assert SHA UnityFramework trước/sau
giữ nguyên; không thêm bước publish release. Kiểm chứng: `python3 -c
yaml.safe_load(...)` OK, 12 steps (tăng từ 11), step mới đúng vị trí; bước
otool chưa chạy thật được ở đây (chỉ có trên macOS runner) — CI mới chạy.

**Kiểm chứng chung sau sửa:** `python3 -m py_compile` cho cả 3 script OK;
`grep -rni` chuỗi mã IAP trong repo chỉ còn trúng 2 file báo cáo (.md) nhắc tới
chuỗi này bằng lời (HANDOFF-SENSOR.md dòng 172 và AEGIS-REPORT.md) — không có
trong `src/`, `scripts/`, workflow hay JSON; IPA gốc sau mọi bước vẫn
SHA-256 `65f89d34…` (không bị sửa); scratch verify đã xóa.
