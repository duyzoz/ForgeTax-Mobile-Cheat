# AEGIS REPORT — Independent QA: BUILD CHEAT FORGETAX iOS

- **Ngày verify:** 2026-11-22 (buổi tối, giờ VN). Verifier: AEGIS (độc lập, không viết code, không sửa gì trong repo).
- **Repo verify:** `~/workspace/forgetax-ios/cheat-build/` (đọc toàn bộ `src/`, `scripts/`, workflow, README, HANDOFF-SENSOR.md, `patch_table_verified.json`).
- **IPA gốc (chỉ đọc, không sửa):** `~/workspace/browser_downloads/sess-124068829/browser-download-20261122T111630.179864570Z-0-com.soucean.fruityzoo_v3.16.1_und3fined.ipa`
  - SHA-256 IPA (tự tính, khớp HANDOFF): `65f89d3496820087b9476c826c0692ecca8b61c45dbe8a05a78d42f44af87cdf`
  - UnityFramework SHA-256 (tự tính từ IPA giải nén): `40a7f74082350afc2312b4c6e9fc224258e8900b01051825393936714c17f158`, 119,049,072 B — khớp JSON meta.
  - IPA gốc sau mọi bước vẫn `65f89d34…` (không bị sửa).
- **Verdict: PASS_WITH_PROOF — kèm 5 mục nên sửa (A–E dưới đây). Không phát hiện vi phạm luật cứng nào (không IAP string, không sửa save/asset, không hook render loop, không timer < 1s chạy hoài, patch table đúng).**

---

## 1. Patch table vs UnityFramework thật (độc lập, 3 chiều)

**PASS.** Tự giải nén IPA ra scratch riêng, đọc bytes gốc tại từng RVA (RVA == file offset), đối chiếu JSON ↔ file thật ↔ `src/patch_table.h`.

| Kiểm tra | Kết quả |
|---|---|
| Tổng entries JSON | 58 = 20 patch áp dụng được (14 MATCH + 6 DERIVED) + 37 call CHECKED + 1 MISMATCH |
| Original bytes khớp file thật (tự đọc, cả 58 entry) | **58/58 khớp, 0 lệch** |
| Tính năng #2–#40 được bao phủ | **39/39, không thiếu tính năng nào** (mỗi feature ≥ 1 entry) |
| Patch site áp dụng được | 20 site, thuộc 19 tính năng (#5 có 2 site) |
| #4 Guard | Chỉ `0x2751574` (`csel`→`mov w0,w8`); patch cũ `MOV w0,#3; RET` tại `0x2751550` **không có trong mảng áp dụng** — chỉ xuất hiện trong comment giải thích ở `patch_table.h:88` |
| #14 OfferPrice | Chỉ `0x275D430` trong bảng áp dụng; `0x275D428` MISMATCH **không** nằm trong mảng áp dụng, chỉ trong comment "DISABLED entries" cuối header |
| `src/patch_table.h` sinh lại từ JSON | **Byte-identical** (`python3 scripts/patch_table_to_header.py …` → `diff` không khác gì) |
| Capstone tự decode 20 patched bytes (capstone tự cài) | **20/20 decode đúng như notes trong JSON** (vd #4 `mov w0, w8`; #5 `fmov d0, xzr; ret` + `mov w0, #0; ret`; #11 stub 20B `ldr…; mov w1,#1; br x3`; #17 `mov w0, #0x63; ret; nop`; #39 `fmov d1, #10.0`) |

Lệnh đã chạy:
```
python3 scripts/verify_patch_table.py patch_table_verified.json --unity <UnityFramework>
# → "UnityFramework SHA-256: 40a7f740… / SHA matches JSON meta. / entries: 20 patch applyable, 37 call (reference), 1 disabled / All applyable original bytes match the binary. / Stale address 0x275D428 is disabled (MISMATCH, do_not_apply=true). OK. / VERIFY OK" (exit 0)
python3 scripts/patch_table_to_header.py patch_table_verified.json /tmp/aegis-header2.h && diff -q src/patch_table.h /tmp/aegis-header2.h
# → identical
```

**Hạn chế của chính `verify_patch_table.py` (xem mục lỗi B):** script chỉ tự kiểm tra 20/58 entry (bỏ qua toàn bộ 37 call entry) và docstring của nó mô tả một check (regen header + grep stale RVA) mà code thực tế **không làm**. Tôi đã bù bằng check độc lập 58/58 ở trên, nên kết luận PASS không dựa vào script này.

## 2. Dry-run pipeline đóng gói bằng dylib GIẢ

**PASS** (toàn bộ trên **bản copy** của IPA; IPA gốc không chạm tới).

Lệnh đã chạy (dylib giả = file rỗng 0 byte tên `ForgeTaxCheat.dylib`):
```
cp <ipa gốc> ~/workspace/aegis-verify/base-copy.ipa   # SHA copy = 65f89d34… (khớp gốc)
unzip -q base-copy.ipa                                # vào ~/workspace/aegis-verify/work/
python3 scripts/inject_dylib.py work/Payload/ForgeTazBlacksmithLegends.app "@executable_path/Frameworks/ForgeTaxCheat.dylib"
# → "Added LC_LOAD_DYLIB @executable_path/Frameworks/ForgeTaxCheat.dylib … / UnityFramework unchanged: 40a7f740…" (exit 0)
cp ForgeTaxCheat.dylib work/Payload/ForgeTazBlacksmithLegends.app/Frameworks/ForgeTaxCheat.dylib
zip -r -y ForgeTax-Cheat-DRYRUN.ipa Payload            # 147,823,552 B
unzip -t ForgeTax-Cheat-DRYRUN.ipa                     # "No errors detected"
```

| Kiểm chứng | Kết quả |
|---|---|
| (a) SHA-256 UnityFramework sau inject + sau copy dylib | `40a7f740…` **không đổi** (before = after, `diff` sạch) |
| (b) LC_LOAD_DYLIB `@executable_path/Frameworks/ForgeTaxCheat.dylib` trong exe chính | **Có** — LIEF parse độc lập: 48 commands (gốc 47), command mới cuối cùng là `TYPE.LOAD_DYLIB @executable_path/Frameworks/ForgeTaxCheat.dylib`. Lưu ý: filter `c.command.name=="LC_LOAD_DYLIB"` của tôi lúc đầu trả 0 là do tên enum LIEF 1.0 là `LOAD_DYLIB`, không phải `LC_LOAD_DYLIB` — đã kiểm lại bằng cách liệt kê toàn bộ commands so với exe gốc (pristine SHA `15f3bdb2…`) |
| (c) Dylib giả đúng vị trí | `Payload/ForgeTazBlacksmithLegends.app/Frameworks/ForgeTaxCheat.dylib` (0 B) có trong IPA đóng gói lại |
| (d) Info.plist nguyên vẹn | SHA `911e4255c4694ac8053aebdedc8632e6643aef296cbfd3dd7594169adb42e696` trước = sau |
| (e) Cấu trúc Payload / zip mở được | `unzip -t` OK, không lỗi; `Payload/ForgeTazBlacksmithLegends.app` + `Payload/decrypt.day` (0 B, có sẵn trong IPA gốc) đầy đủ |
| Exe chính sau inject | SHA đổi `15f3bdb2…` → `9584f594…`, size 88,608 → 88,496 B (LIEF rewrite, kèm mất hiệu lực code signature gốc — chờ bước `ldid` trong workflow ký lại, đúng thiết kế) |

- LIEF cài được: `pip install --break-system-packages lief` → **lief 1.0.0** (lần đầu `pip install lief` fail do PEP 668 externally-managed; dùng `--break-system-packages` thì được). Capstone cũng cài được tương tự (dùng cho mục 1).
- Sự cố môi trường: `/tmp` là tmpfs 512 MB, đầy 100% khi giải nén IPA lần đầu (`cp: No space left on device`); đã dọn `/tmp/aegis-*` và chuyển scratch sang `~/workspace/aegis-verify/` (đã xóa sau khi xong; mọi hash/bằng chứng giữ trong báo cáo này).
- **Phát hiện trong lúc dry-run (xem mục lỗi A):** chạy `inject_dylib.py` lần 2 trên cùng exe **không** báo "already present" mà thêm **trùng** LC_LOAD_DYLIB (49 commands, 2 entry giống hệt). Nguyên nhân: check idempotency dùng sai tên enum LIEF 1.0. Workflow CI chỉ chạy 1 lần từ IPA giải nén mới nên không bị, nhưng chạy lại local thì bị.

## 3. Rà soát code dylib

### (a) String mã sản phẩm IAP — PASS
- `grep -rniE` chuỗi mã IAP toàn repo: **0 lần xuất hiện**. (Trong tài liệu gốc ngoài repo cheat thì có thật 2 product id — và code không chứa chúng.)
- Không có StoreKit / `SKPayment` / restore-purchase / product-id nào trong `src/`. Các method IAP-adjacent trong game (`GrantNoAds` 0x263EDB4, `RevokeNoAds` 0x263EEF0, `TogglePurchaseShelfSlot` 0x263F1CC, `PurchaseStatus` — đều có thật trong `dump.cs`) **không được gọi ở đâu trong code**; chỉ README dòng 43 nhắc tên chúng trong câu "Không làm".
- `com.soucean.fruityzoo` xuất hiện (bundle id trong comment + URL workflow) — hợp lệ, không phải IAP string.

### (b) Sửa save/asset — PASS
- Không có `writeToFile` / `NSFileManager` write / `fopen` / `ofstream` / `PlayerPrefs` / `NSUserDefaults set` / asset modification nào trong `src/`. Lần grep duy nhất trúng là comment cấm trong `GameBridge.h:22`.
- Không có network call nào (`NSURLSession`/`curl`/`socket`/http — grep 0 kết quả): dylib không exfiltrate gì.

### (c) Hook render loop — PASS
- Không có swizzle / `method_exchange` / hook Unity render / `GLKView` / `MTKView` / `OnGUI` / `Update()` nào trong `src/` (grep 0 kết quả). UI chỉ rebuild khi user tương tác (`rebuildContent` trong các handler).

### (d) Timer — PASS (đúng như mô tả)
- `ForgeTaxCheat.mm:87`: **1 NSTimer duy nhất**, interval **1.0 s**, `repeats:YES`, **tự dừng** khi attach thành công (`FTC_TryAttach` invalidate) hoặc sau **tối đa 60 lần** (`s_attachTries >= 60`). Không timer nào < 1 s, không timer nền nào sống sau attach.
- `CheatMenuView.mm:852–892`: **CADisplayLink duy nhất**, chỉ tạo trong `setFpsBadgeVisible(YES)` (tức chỉ khi #40 bật), `invalidate` khi tắt + trong `dealloc`. Callback `onFpsTick` chạy mỗi frame nhưng chỉ tăng counter; **label chỉ cập nhật khi `now - lastUpdate >= 1.0` s** (đúng 1 Hz). Không làm việc gì với game trong callback.
- Không `dispatch_source` timer, không `dispatch_after` lặp nào.

### (e) Verify original bytes trước khi ghi — PASS
- `PatchEngine.mm` `FTC_WriteVerified`: `memcmp(addr, bytes)==0` → ALREADY; `memcmp(addr, expected)!=0` → `FTC_PATCH_ERR_MISMATCH`, **từ chối ghi**; chỉ sau đó mới `vm_protect` RW → `memcpy` → `sys_icache_invalidate` → protect lại R+X. Apply/remove đều qua hàm này; mọi entry đều qua `FTC_StatusApplyable` (MATCH/DERIVED/CHECKED — xem lưu ý C về `PROVISIONAL`).
- Ghi chú: `vm_protect` được gọi với địa chỉ/len đúng bằng patch (4–20 B, không page-align) — kernel Mach làm tròn về page nên kỹ thuật này là chuẩn cho runtime patch, nhưng **chưa có bằng chứng chạy thật**; chỉ CI/iPhone mới trả lời (ghi ở mục chưa verify).

### (f) Null-check / try-catch mọi call vào game — PASS
- `GameBridge.mm`: `Api()->loaded` check trước mọi call; `klass==NULL` → `NO_METHOD`; `method==NULL` → `NO_METHOD`; `runtimeInvoke` bọc `@try/@catch`; `exc` được kiểm tra; `domain==NULL`, `assemblies==NULL`, `image==NULL`, `tools==NULL` đều return sớm. `ResolveTools` null-check từng mắt xích (pause → page → tools; fallback room → ctor → tools). `InvokeBool` xử lý cả method void (`ret==NULL` → OK) và bool (unbox, false → `RETURNED_FALSE`).
- `ForgeTaxCheat.mm` (4 `@try`), `CheatMenuView.mm` (13 `@try`), `PatchEngine.mm` (2 `@try` bọc apply/remove) — mọi entry đều bọc.
- **Tự kiểm chứng thêm bằng `dump.cs`** (ngoài phạm vi bắt buộc nhưng tăng độ tin cậy): cả 14 symbol IL2CPP mà `GameBridge.mm` dlsym đều **có trong exported symbols của UnityFramework** (LIEF: 15,349 exported, gồm `_il2cpp_domain_get`, `_il2cpp_runtime_invoke`, … — lần grep `strings` đầu tiên trả rỗng chỉ vì symbol Mach-O có prefix `_`).
- Chuỗi instance `ForgePauseMenu` (`ForgeTax.Menu`) → field `developerPage` (0x20) → `ForgeDeveloperPage` (`ForgeTax.Touch`) → field `tools` (0x10) → `ForgeDeveloperTools` → field `room` (0x10) → `ForgeRoom` (`ForgeTax.Room`) → field `campaign` (0x2A8) → `ForgeCampaign` (`ForgeTax.Rules`): **khớp hết với `dump.cs`**. Enum `ForgeOutcome` khớp: Damage=1, Hold=2, Success=3, GreatSuccess=4 (dùng ở #12).
- Tất cả method dev-tool mà GameBridge gọi đều có trong `dump.cs` đúng RVA trong JSON: AddGold 0x263BF68, GiveAllRelics 0x263D5A4, CycleForgeMode 0x263DAF0, SetAnvilGrade 0x263C178, ToggleGodMode 0x263DC74, KillDragon 0x263E578, SummonMerchant 0x263E9D8, JumpToDay 0x263B8B8, FillFever 0x263D424, AddSwordCredits 0x263C7AC, GiveSecretDoorKey 0x263CE68, EnterSkillArena 0x263DF6C, OpenSwordPicker 0x263C8B4, RevealCodex 0x263F7F4, ResetRun 0x263F6AC, SkipDay 0x263D1A4, NextDay 0x263D34C, StartDragonFight 0x263DDD4, ForceDragonLoss 0x263E660, OpenArmory 0x263E748, ToggleMerchantShelfSlot 0x263F02C, ToggleFps 0x263EC68, SetQualityTier(Nullable<QualityTier>) 0x263C410, `.ctor(ForgeRoom)` 0x263C4E8, AddRunTickets(int,int) 0x27588A8 (void). Tất cả đều trả `bool` hoặc `void` — khớp logic unbox trong `InvokeBool`.
- Các RVA call còn lại trong JSON cũng khớp tên method trong `dump.cs` (HatchCocoon 0x275AF4C, TryFuse 0x275A710, UseConsumable 0x2753340, RewindAfterDragonWin 0x2753C98, OwnsDragonTrophy 0x275F474, Unlock 0x275F938, GrantHellDrop 0x26A4CFC, Chest 0x26C717C, ClaimHellDrop 0x275F90C, SwordValuation.Value 0x276F248, SetGrade 0x2646898, Pick 0x26469A0, GrantCreditShelfSlot 0x2758A2C). Lưu ý: 0x275F938 trong dump.cs là `private void Unlock(DragonTrophy)` (tên JSON "UnlockDragonTrophy" là tên gọi tắt) và GameBridge **không gọi trực tiếp** các RVA này — chúng chỉ là tham chiếu; #37 chỉ dùng GiveAllRelics.

### (g) `src/patch_table.h` sinh từ JSON — PASS
- Regen byte-identical (mục 1). 20/20 entry áp dụng trong header khớp JSON (RVA + original + patched + len + status + feature). Entry MISMATCH `0x275D428` không nằm trong mảng áp dụng. Bytes patch cũ #4 chỉ trong comment.

### (h) Workflow YAML — PASS (với 1 thiếu sót nhỏ, xem E)
- URL base IPA đúng như README: `https://github.com/duyzoz/ForgeTax-Mobile-Cheat/releases/download/base-ipa-3.16.1/com.soucean.fruityzoo_v3.16.1_und3fined.ipa`; fail rõ ràng nếu asset chưa có ("Upload … then re-run", "No other download source is used").
- Assert SHA UnityFramework trước inject (`unity-before.txt`) và sau inject (`unity-after.txt` + `diff`, fail nếu khác) — có.
- **Không có bước publish release** — chỉ `actions/upload-artifact@v4` (IPA + `.sha256`). YAML parse OK (11 steps, 1 job `build`, `workflow_dispatch`).
- `ldid` best-effort đúng mô tả: `brew install ldid || true`, `ldid -S` có `|| echo ::warning::…`, re-pack sau khi ký, cập nhật lại `.sha256`.
- Header sync check có (`patch_table_to_header.py` → `diff`, fail nếu lệch). `scripts/*.py` compile OK (`py_compile`).
- Thiếu: không có bước assert LC_LOAD_DYLIB thực sự có trong IPA cuối (xem E).

### Đối chiếu FeatureCatalog ↔ patch table ↔ UI handler — PASS
- Catalog: **39 entry, fid 2–40 đầy đủ, không trùng, không thiếu**.
- PATCH-kind trong catalog (19 fid: 4, 5, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 34, 35, 36, 38, 39) **khớp chính xác** 19 fid có patch site trong JSON.
- DEV_TOGGLE (7, 33, 40), ONESHOT (2, 3, 8, 23, 24, 25, 26, 27, 29, 30, 31, 37), STEP (6, 9, 12), MODE (28, 32) — mọi fid đều có handler trong `CheatMenuView.mm` (`runOneshot` switch đủ 12 oneshot; `onSwitch` đủ patch/dev-toggle; `onStepApply` 6/9; `onTicketPlus` 12; `onSegment` 28/32). #27 có alert xác nhận trước khi ResetRun (`CheatMenuView.mm` ~726). #10 khi bật gọi FillFever 1 lần rồi mới báo trạng thái.
- Nhóm CHEAT ENGINE trong menu thật đếm động = 37 (non-setting) — đúng, không hard-code 38 như mockup.

## 4. Đối chiếu mockup vs README — XÁC NHẬN con số của SENSOR

**SENSOR nói đúng hoàn toàn.** Đếm trực tiếp trong `~/workspace/forgetax-ios/mockup/cheat-menu-mockup.html` (525 dòng):

- Mảng `F` = **38 entry, id 2–39 đầy đủ, không thiếu** (đếm bằng regex trên đúng block `var F = [...]`).
- `FPS_ROW` riêng = **id 40**.
- Tổng mockup = 38 + 1 = **39 tính năng**, khớp README (39 tính năng #2–#40).
- Nhóm CHEAT ENGINE render `F` lọc theo 7 cat (forge/res/relic/shop/dragon/day/ads) — **#32 (cat `setting`) bị loại → render 37 mục**, trong khi header mockup (dòng 392, 395) hard-code **"38 tính năng"** → **nhãn header của mockup sai 1** (38 thay vì 37). Đây là lỗi của mockup, không phải của menu thật.
- Nhóm SETTING render `FPS_ROW` + các `F` cat setting = **2 mục (#40 + #32)** — header "2 mục" đúng.
- Menu thật (`CheatMenuView.mm` `rebuildContent`) đếm động 37 cho CHEAT ENGINE và hard-code "2 muc" cho SETTING — hiển thị đúng.

## Lỗi tìm thấy (xếp theo mức độ)

### BLOCKER — không có
Không phát hiện vi phạm luật cứng nào: không IAP string, không sửa save/asset, không hook render loop, không timer < 1 s chạy hoài, patch table đúng 58/58 bytes, header đồng bộ, packaging dry-run đạt.

### NÊN SỬA (5 mục — khuyến nghị sửa trước khi Ethan test live, nhưng không chặn chạy CI)

**A. `scripts/inject_dylib.py:63` — check idempotency chết dưới LIEF 1.0.0 → chạy lại sẽ thêm TRÙNG LC_LOAD_DYLIB (đã chứng minh).**
Bằng chứng: `cmd.command.name` thực tế là `'LOAD_DYLIB'`, không phải `'LC_LOAD_DYLIB'` như code so sánh; chạy script lần 2 trên cùng exe in "Added LC_LOAD_DYLIB …" lần nữa, LIEF đếm **49 commands, 2 entry `@executable_path/Frameworks/ForgeTaxCheat.dylib` giống hệt**. CI chạy 1 lần từ IPA giải nén mới nên không bị; chạy tay/re-run thì bị. Sửa: so sánh với `"LOAD_DYLIB"` (hoặc kiểm tra `load_path in str(cmd.name)` trên các command loại dylib).

**B. `scripts/verify_patch_table.py` — docstring nói quá + bỏ qua 37 call entry.**
- Docstring dòng 9–11 / 15–16 mô tả check 3 là "re-generating the header in memory and grepping for the stale RVA" — code thực tế **không hề đọc `src/patch_table.h`**, chỉ kiểm tra các entry trong JSON có `do_not_apply`. (Workflow bù lại bằng bước diff header riêng, nên drift header vẫn bị bắt — nhưng docstring gây hiểu lầm về coverage.)
- Vòng lặp chính `continue` ngay với `kind == "call"` — **37/58 entry không bao giờ được đối chiếu bytes** bởi chính script này. Tôi đã tự đối chiếu 58/58 và đều khớp, nhưng script nên tự làm việc đó. Sửa: thêm check original bytes cho call entries + sửa docstring cho đúng code (hoặc implement check 3 thật).

**C. `src/PatchEngine.mm:58–62` — `FTC_StatusApplyable` chấp nhận status `"PROVISIONAL"`.**
Hiện không có entry PROVISIONAL nào trong bảng (chỉ MATCH/DERIVED) và comment struct trong `patch_table.h` cũng chỉ ghi `"MATCH" | "DERIVED" | "CHECKED"` — nhưng allowlist này là một lỗ hổng tiềm ẩn: nếu một entry provisional lọt vào JSON/table sau này, nó sẽ được apply mà không qua verify. Sửa: bỏ `"PROVISIONAL"` khỏi allowlist.

**D. `src/GameBridge.mm:145–146,171,213` — `s_toolsCache` / `s_campaignCache` không bao giờ được invalidate.**
`ResolveTools()`/`ResolveCampaign()` return cache mãi mãi sau lần resolve đầu. Nếu game destroy/recreate pause menu hoặc scene reload làm tools/campaign object cũ bị hủy, các lần gọi sau sẽ invoke trên con trỏ treo (dangling `Il2CppObject*`) → nguy cơ crash thay vì chỉ báo `NO_INSTANCE`. SENSOR đã ghi fallback "chưa kiểm chứng live"; riêng cache treo này là rủi ro crash thực sự. Sửa: re-resolve mỗi lần gọi (hoặc validate cache / clear khi scene đổi).

**E. `.github/workflows/build-cheat-ipa.yml` — không assert LC_LOAD_DYLIB có trong IPA cuối.**
Workflow assert SHA UnityFramework (tốt) nhưng không kiểm tra gì về load command sau inject — nếu `inject_dylib.py` fail im lặng hoặc LIEF đổi API, CI vẫn upload IPA không có cheat (script có exit nonzero khi fail rõ, nhưng không có assert độc lập). Sửa: thêm step `otool -l work/$APP/<exe> | grep -q ForgeTaxCheat` (hoặc parse bằng LIEF) sau bước inject.

### GHI CHÚ (không phải lỗi)
- **#28 "Nhảy ngày ±1":** tôi đã nghi ngờ seg `UISegmentedControl` không fire khi bấm cùng phía 2 lần liên tiếp — **kiểm tra lại thì KHÔNG phải bug**: `onSegment` luôn gọi `rebuildContent` nên control được tạo lại với `selectedSegmentIndex = -1` (mặc định không chọn gì) sau mỗi lần bấm, vì vậy lần bấm sau luôn là một "change" và fire bình thường. Chỉ có hệ quả thẩm mỹ: seg không giữ trạng thái chọn sau khi bấm (chấp nhận được vì #28 là action-per-tap, không phải mode).
- **`#27` alert:** nếu `keyWindow.rootViewController` là nil thì alert không hiện và ResetRun không chạy, user bấm mà không thấy gì (`CheatMenuView.mm:733`). Trường hợp hiếm; có thể thêm fallback present từ window.
- **DEV_TOGGLE desync:** menu giả định flag game OFF lúc launch (ghi rõ trong `FeatureCatalog.h`); nếu Ethan đã bật god mode/shelf/FPS từ dev page trong game trước, 1 lần gạt menu sẽ tắt game nhưng hiển thị bật. Edge case, đã được disclose.
- **#10 Fever:** nếu `FillFever()` fail khi bật, patch Expire vẫn ở trạng thái bật (status text có ghi chú "Patch bat; …"). Hành vi này hợp lý (patch mới là phần chính).
- **#33 (credit-shelf):** `ToggleMerchantShelfSlot` (0x263F02C) có thật trong `dump.cs`; nhưng `GrantCreditShelfSlot` (0x2758A2C) tồn tại riêng trong game và **không được gọi** — liệu toggle có thực sự cấp ô hay chỉ lật cờ UI thì chỉ test live mới trả lời. Tương tự #12 (chuỗi tools→room→campaign), #32 (layout `Nullable<QualityTier>`), fallback `.ctor` — tất cả đều đã được SENSOR ghi là chưa kiểm chứng live.
- **LIEF rewrite:** exe chính thu nhỏ 88,608 → 88,496 B và code signature gốc mất hiệu lực (dự kiến; bước `ldid` trong workflow ký lại).
- **#37:** không có call riêng, dùng chung `GiveAllRelics()` như #3 — đúng theo README (game tự nhận cúp khi đủ di vật cup).

## CHƯA VERIFY ĐƯỢC Ở ĐÂY
1. **Biên dịch thật** — không có macOS/Xcode trong sandbox này; mọi lỗi cú pháp ObjC++ chỉ CI mới bắt được (đã kiểm tra tĩnh: `py_compile` cho scripts OK, YAML parse OK, brace/paren và `@selector` đã được SENSOR kiểm tra — tôi không biên dịch lại được).
2. **Chạy thật trên iPhone** — chỉ Ethan làm được: app có launch không, logo có hiện không, instance IL2CPP có resolve không, patch có ăn không, FPS có tụt không, fallback `.ctor` có chạy không, #12/#32/#33 có tác dụng không, `vm_protect` không page-align có chạy trên iOS thật không.
3. **Release asset base IPA** (`duyzoz/ForgeTax-Mobile-Cheat`, tag `base-ipa-3.16.1`) — chưa kiểm tra có tồn tại trên GitHub chưa; workflow sẽ fail rõ ràng nếu chưa có (theo thiết kế).
4. **Semantics sâu của từng patch** (vd #11 stub có gọi đúng `finished(true)` trong ngữ cảnh thật không, #13 branch target có skip đúng phép trừ ví không) — chỉ mới verify ở mức: original bytes khớp file thật + patched bytes decode đúng bằng capstone + notes khớp `dump.cs`. Hành vi runtime chỉ test live mới trả lời.
5. **Dylib thật** — dry-run dùng file rỗng 0 byte (theo brief); việc dylib thật có load được qua `LC_LOAD_DYLIB` + `ldid` trên TrollStore hay không chỉ CI + iPhone mới trả lời.

## Tóm tắt lệnh đã chạy (bằng chứng)
```
sha256sum <ipa gốc>                                   → 65f89d34… (khớp)
sha256sum <UnityFramework trong IPA giải nén>         → 40a7f740…, 119,049,072 B (khớp)
python3 scripts/verify_patch_table.py … --unity …     → VERIFY OK, exit 0
python3 scripts/patch_table_to_header.py … + diff     → header identical
check độc lập 58 entry (script /tmp/aegis_check.py)   → 58/58 bytes khớp, features 2–40 đủ
capstone decode 20 patched bytes                      → 20/20 đúng như JSON notes
python3 scripts/inject_dylib.py <copy .app> …         → Added LC_LOAD_DYLIB, UnityFramework unchanged
LIEF parse exe sau inject vs pristine                 → 48 vs 47 commands, load command mới có mặt
LIEF parse sau chạy inject lần 2                      → 49 commands, 2 entry trùng (lỗi A)
sha256sum UnityFramework / Info.plist trước–sau       → không đổi
zip -r -y + unzip -t ForgeTax-Cheat-DRYRUN.ipa        → OK, 147,823,552 B
LIEF exported symbols UnityFramework                  → 14/14 il2cpp_* symbols có mặt
grep dump.cs: methods/RVA/namespace/field offsets     → khớp hết với GameBridge
grep -rniE 'IAP|StoreKit|…' / mockup count / catalog cross-check / py_compile / yaml parse
```
