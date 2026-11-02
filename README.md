# mcpelauncher-imghost — "I'm Ghost"

Mod untuk **MCPELauncher** (Unofficial \*NIX Launcher for Minecraft). Dengan mod ini pemain bisa berubah menjadi **hantu**: mode spectator yang bisa menembus dinding. Mode ini dipasang langsung ke pemain (bukan lewat command), sehingga **tidak membutuhkan cheat aktif** dan dapat dipakai di dunia **Hardcore**.

Struktur project mengikuti [mcpelauncher-cheatlesstweaks](https://github.com/Kashifro/mcpelauncher-cheatlesstweaks) (CMake + Android NDK, hasil berupa `.so` di folder `mods`), tetapi fungsinya berbeda.

## Cara pakai

| Aksi | Cara |
|------|------|
| Masuk / keluar mode hantu | **Alt+X** (default) |
| Ganti hotkey | Menu launcher → **I'm Ghost** → *Set hotkey*, lalu tekan kombinasi baru |
| Toggle lewat menu | Menu launcher → **I'm Ghost** → *Ghost mode* |
| Posisi saat keluar | Menu → *Return to start position on exit* |

Posisi saat kembali menjadi pemain:

- **Default (`stay`)**: pemain berada di posisi terakhir saat masih menjadi hantu.
- **`origin`**: pemain dikembalikan ke posisi saat mode hantu dimulai.

Peringatan: pada mode `stay`, keluar dari mode hantu saat berada di dalam blok bisa membuat pemain tercekik. Di Hardcore itu berarti mati permanen. Keluarlah di tempat yang terbuka.

## Build

Syarat: Android NDK r27+ dan CMake 3.15+. Tidak ada dependensi eksternal (tidak memakai libhat/safetyhook/zydis; hook memakai API bawaan launcher).

```
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/ndk/build/cmake/android.toolchain.cmake -DANDROID_ABI=x86_64 -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

Salin `libmcpelauncher-imghost.so` ke folder `mods` milik launcher.

## Konfigurasi (`imghost.cfg`)

File dibuat otomatis di sebelah `.so` saat hotkey/opsi diubah lewat menu. Bisa juga diedit manual:

```
hotkey_key=88        # kode tombol (88 = X)
hotkey_alt=1
hotkey_ctrl=0
hotkey_shift=0
exit_mode=stay       # stay | origin
debug_keys=0         # 1 = tulis kode tombol ke logcat
```

Opsi lanjutan (tidak ditulis otomatis): `key_alt_l`, `key_alt_r`, `key_ctrl_l`, `key_ctrl_r`, `key_shift_l`, `key_shift_r`, `action_press`, `action_release`, `action_repeat`, `gametype_spectator`, `gametype_survival`, dan `sym_tick`, `sym_get_pos`, `sym_set_pos`, `sym_get_gametype`, `sym_set_gametype`.

## Hal yang HARUS diverifikasi sebelum dipakai

Kode ini belum pernah dijalankan atau dikompilasi dengan NDK; belum ada binary game untuk mengujinya. Bagian berikut berasal dari dokumentasi atau ingatan, bukan dari pengujian:

1. **Nama simbol game.** Default (`LocalPlayer::normalTick`, `Actor::getPosition`, `Actor::setPos`, `Player::getPlayerGameType`, `Player::setPlayerGameType`) adalah tebakan berdasarkan konvensi Bedrock dan bisa berbeda per versi. Cek dengan:
   `nm -D libminecraftpe.so | grep -E "LocalPlayer|setPlayerGameType|getPlayerGameType"`
   lalu timpa lewat kunci `sym_*` di `imghost.cfg`. Jika ada yang tidak ditemukan, mod menulis alasannya ke logcat (tag `ImGhost`) dan mode hantu tidak aktif.
2. **Nilai GameType.** Default Spectator = 6, Survival = 0. Ubah lewat `gametype_*` bila versi game berbeda.
3. **Kode tombol.** Default mengikuti layout virtual-key Windows (Alt=18, Ctrl=17, Shift=16, X=88). Setel `debug_keys=1`, tekan tombol, dan baca kode di logcat; sesuaikan bila berbeda.
4. **Entry point `mod_init`.** Dokumentasi Modding API yang tersedia tidak menyebutnya; ini diambil dari ingatan atas kode launcher. Jika logcat tidak menampilkan `I'm Ghost: mod_init`, fungsi itu tidak dipanggil dan perlu dicek di source launcher.
5. **Sisi server.** Di dunia single-player, `setPlayerGameType` pada `LocalPlayer` bisa saja ditimpa oleh pemain sisi server. Jika spectator tidak bertahan atau tidak menembus dinding, hook perlu dipindah ke pemain server atau memakai Abilities (NoClip/Flying).

## Struktur

```
CMakeLists.txt
src/main.cpp          entry point (mod_init)
src/ghost.cpp         hook tick pemain, logika masuk/keluar hantu
src/input.cpp         hotkey Alt+X, rebind, lewat Game Window API
src/menu.cpp          entry menu & jendela rebind, lewat Menu API
src/config.cpp        baca/tulis imghost.cfg
src/launcher_api.h    deklarasi API launcher
```
