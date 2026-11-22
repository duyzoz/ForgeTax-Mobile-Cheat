# ForgeTax iOS 3.16.1 — Cheat (dylib, TrollStore)

Cheat menu cho ForgeTax iOS 3.16.1 (build 95, `com.soucean.fruityzoo`),
chạy bằng dylib tiêm runtime qua TrollStore trên iPhone 7 Plus (iOS 15+).

**Game gốc không bị sửa.** UnityFramework trong IPA đóng gói phải
byte-identical với bản gốc — workflow kiểm tra SHA-256 trước/sau và fail
nếu khác. Mọi tính năng là patch runtime (ghi đè bytes trong RAM khi bật,
khôi phục khi tắt) hoặc gọi hàm dev-tool có sẵn trong game qua IL2CPP.

## Tính năng (39, #2–#40)

Mở menu: bấm logo lửa (kéo để di chuyển). Tất cả mặc định **OFF** khi vào game.

| Nhóm | Tính năng |
|---|---|
| Rèn | Rèn luôn thành công, giữ Great (#4) · Tỉ lệ gãy về 0 (#5) · Grade kiếm trên đe, tối đa 999 (#6) · Fever đầy ngay / không hết (#10) · Nhân giá trị kiếm ×10 (#39) |
| Tài nguyên | Vàng vô hạn 9.999E+99 (#2) · Quota về 1 vàng (#16) · 99 mạng (#17) · +10 Sword Credits (#25) |
| Di vật | Full di vật 119 (#3) · Mở ô di vật bằng vàng, credit-shelf (#33) · Nở kén ngay (#34) · Ép fusion ngay (#35) · Vật phẩm dùng vô hạn (#36) · Mở 3 cúp rồng (#37) · Hell Drop mỗi lần thắng rồng (#38) |
| Thương nhân | Gọi thương nhân bất cứ lúc nào (#8) · Vé merchant +1 (#12) · Mua hàng merchant miễn phí (#13) · Shop di vật miễn phí (#14) · Reroll miễn phí (#15) |
| Rồng | God mode rồng + Giết rồng (#7) · Một phát chết rồng (#18) · Bất tử trong trận rồng (#19) · Rồng choáng vĩnh viễn (#20) · Rồng không nổi giận (#21) · Rồng không bay (#22) · Đánh rồng bất cứ lúc nào (#29) · Ép thua rồng (#30) |
| Ngày & Chơi | Nhảy tới ngày (#9) · Cửa bí mật: +30 kiếm vào tay (#23) · Đấu trường kỹ năng (#24) · Mở Codex (#26) · Chơi lại từ đầu (#27) · Nhảy ngày nhanh ±1 (#28) · Mở kho vũ khí (#31) |
| Quảng cáo | Skip quảng cáo nhận thưởng (#11) — nút quảng cáo vẫn còn nguyên, bấm là nhận luôn, không xem video |
| Setting | Chất lượng đồ họa Thấp / Vừa / Cao (#32) · Hiện FPS (#40) |

Loại tính năng trong menu (xem `src/FeatureCatalog.h`):
- **Toggle patch** — bật ghi bytes, tắt khôi phục bytes gốc. Patch chỉ ghi
  khi bytes gốc tại địa chỉ đó khớp bảng; không khớp thì từ chối và báo lỗi.
- **One-shot** — bấm là gọi hàm dev-tool một lần, không giữ trạng thái.
- **Mode / Stepper** — chọn mode hoặc chỉnh số rồi áp dụng.

## Lưu ý quan trọng

- **#4 Rèn luôn thành công:** patch tại `0x2751574` (`csel` → `mov w0,w8`),
  luôn trả kết quả AlwaysSuccess nhưng **giữ Great** khi input là Great.
  Patch cũ (`MOV w0,#3; RET` tại entry) đã bị loại vì hạ Great xuống
  Success. **Tác dụng phụ:** khi patch #4 đang BẬT, nút "Đổi mode"
  (CycleForgeMode) không còn tác dụng lên Guard nữa — luôn AlwaysSuccess.
  Tắt patch #4 để đổi mode có tác dụng trở lại.
- **#14 Shop di vật miễn phí:** chỉ dùng địa chỉ `0x275D430`. Địa chỉ cũ
  `0x275D428` là sai (đầu hàm, lệnh `ldr`) và đã bị loại khỏi bảng patch.
- **Không làm, theo README gốc §4:** 2 ô di vật trả phí (slot 9/10),
  tăng ô vĩnh viễn lên 11, mua no-ads, xóa/chặn quảng cáo, TestMode,
  sửa asset / text / balance / save. Những thứ này không có trong code.
- Test trên **tài khoản phụ trước**. Save cũ vẫn load được (game tự
  checkpoint sau mỗi thao tác), nhưng "Chơi lại từ đầu" (#27) sẽ mất run
  hiện tại — menu hỏi xác nhận trước khi chạy.
- Không có gì đảm bảo tuyệt đối không crash: patch đã verify bytes, mọi
  lệnh gọi game đều bọc try/catch và kiểm tra null, nhưng đây vẫn là
  cheat tiêm runtime — test kỹ từng tính năng trên tài khoản phụ trước
  khi dùng run chính.

## Cài đặt (TrollStore)

1. Chạy workflow **build-cheat-ipa** (Actions → Run workflow), tải artifact
   `ForgeTax-Cheat-3.16.1.ipa` + file `.sha256`, kiểm tra SHA-256 khớp.
2. Cài IPA qua TrollStore trên iPhone 7 Plus (iOS 15.0–16.6.1).
   Không cần chứng chỉ, không hết hạn 7 ngày.
3. Vào game, bấm logo lửa để mở menu.

Base IPA phải có sẵn dưới dạng release asset `base-ipa-3.16.1` của repo
`duyzoz/ForgeTax-Mobile-Cheat`
(`com.soucean.fruityzoo_v3.16.1_und3fined.ipa`); workflow không tải IPA
từ nguồn nào khác.

## Build tay (macOS + Xcode)

```sh
xcrun --sdk iphoneos clang -arch arm64 -mios-version-min=15.0 \
  -dynamiclib -fobjc-arc -O2 \
  -install_name @executable_path/Frameworks/ForgeTaxCheat.dylib \
  -framework Foundation -framework UIKit \
  -framework CoreGraphics -framework QuartzCore \
  src/ForgeTaxCheat.mm src/CheatMenuView.mm \
  src/PatchEngine.mm src/GameBridge.mm \
  -o ForgeTaxCheat.dylib
```

## Bảng patch — đã VERIFIED

`src/patch_table.h` được sinh từ `patch_table_verified.json` (CIPHER verify
độc lập bằng capstone 5.0.7 trên IPA gốc):

- 58 entries: **20 patch áp dụng được** (14 MATCH + 6 DERIVED) +
  37 call target CHECKED (chỉ để tham chiếu cho GameBridge) +
  **1 MISMATCH** (`0x275D428`, `do_not_apply=true`, không có trong bảng C).
- UnityFramework SHA-256:
  `40a7f74082350afc2312b4c6e9fc224258e8900b01051825393936714c17f158`
  (119.049.072 byte, cryptid=0, RVA == file offset).
- IPA gốc SHA-256:
  `65f89d3496820087b9476c826c0692ecca8b61c45dbe8a05a78d42f44af87cdf`.

Sinh lại header sau khi sửa JSON:

```sh
python3 scripts/patch_table_to_header.py patch_table_verified.json src/patch_table.h
python3 scripts/verify_patch_table.py patch_table_verified.json --ipa <base.ipa>
```

## Cấu trúc repo

```
src/
  ForgeTaxCheat.mm     constructor + gắn overlay vào key window
  CheatMenuView.h/.mm  logo lửa + panel (theo mockup, font hệ thống)
  PatchEngine.h/.mm    patch runtime: verify bytes gốc -> ghi -> khoi phuc
  GameBridge.h/.mm     goi ham dev-tool qua IL2CPP runtime_invoke
  FeatureCatalog.h     metadata 39 tinh nang (loai, mo ta, cach lam)
  patch_table.h        BANG PATCH DUY NHAT (sinh tu JSON, khong sua tay)
scripts/
  patch_table_to_header.py  JSON -> patch_table.h
  verify_patch_table.py     kiem tra lai original bytes voi IPA goc
  inject_dylib.py           chen LC_LOAD_DYLIB (LIEF) — workflow dung
.github/workflows/build-cheat-ipa.yml   build dylib + dong goi IPA (khong publish release)
patch_table_verified.json   bang CHUAN (CIPHER)
```

## Hiệu năng

Không hook render loop, không làm việc mỗi frame. UI chỉ cập nhật khi
tương tác. Lúc mới vào game chỉ có một timer nhẹ 1 giây để chờ key window,
tự dừng khi đã gắn. FPS badge (#40) dùng CADisplayLink riêng, chỉ tồn tại
khi bật, gom 1 giây mới cập nhật label một lần.
