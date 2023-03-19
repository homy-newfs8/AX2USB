# AX2USB
AXキーボード用のPS/2 to USB変換器

## 概要
PS/2接続のAXキーボードをUSBに変換する変換器です。これまで持っていた変換器ではAXキーボードの一部キー(AX、無変換、変換)がWindowsに伝わっていなかったので対応させました。なんとなくうまくいったので機能追加してあります。

## 動作するハードウェア

本ソフトはRP2040ベースのArduinoが使えるボードであればほぼそのまま使えるはずです。RP2040依存なのは`mutex.hpp`の排他制御クラスのみです。[TinyUSB for Arduino](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)がサポートするマイクロコントローラベースのボードであれば移植は簡単でしょう。

### 構成例

* [Seeed XIAO RP2040](https://www.seeedstudio.com/XIAO-RP2040-v1-0-p-5026.html)
* [４ビット双方向ロジックレベル変換モジュール](https://akizukidenshi.com/catalog/g/gK-13837/)
* [ミニＤＩＮコネクタピッチ変換キット](https://akizukidenshi.com/catalog/g/gK-09483/)
* 万能基板

![京セラキーボード](https://raw.githubusercontent.com/homy-newfs8/AX2USB/contents/kbd-kyocera.jpg)
*レアな京セラキーボード*

## 追加してある機能

* <kbd>無変換</kbd>、<kbd>変換</kbd>は<kbd>左Win</kbd>、<kbd>右Win</kbd>として動作
* <kbd>Caps Lock</kbd>は<kbd>Shift</kbd>中のみCaps Lockとして動作
* <kbd>Caps Lock</kbd>(シフトなし)、<kbd>英数カナ</kbd>は<kbd>Fn</kbd>として動作
  * システムコントロール
    * <kbd><kbd>Fn</kbd>+<kbd>AX</kbd></kbd>電源
    * <kbd><kbd>Fn</kbd>+<kbd>Pause</kbd></kbd>スリープ
  * メディアコントロール
    * <kbd><kbd>Fn</kbd>+<kbd>スペース</kbd></kbd>再生/ポーズ
    * <kbd><kbd>Fn</kbd>+<kbd>5</kbd>(テンキー)</kbd>再生/ポーズ
    * <kbd><kbd>Fn</kbd>+<kbd>Home</kbd></kbd>再生
    * <kbd><kbd>Fn</kbd>+<kbd>End</kbd></kbd>ポーズ
    * <kbd><kbd>Fn</kbd>+<kbd>←</kbd></kbd>前トラック
    * <kbd><kbd>Fn</kbd>+<kbd>→</kbd></kbd>次トラック
    * <kbd><kbd>Fn</kbd>+<kbd>↑</kbd></kbd>ボリュームアップ
    * <kbd><kbd>Fn</kbd>+<kbd>↓</kbd></kbd>ボリュームダウン
    * <kbd><kbd>Fn</kbd>+<kbd>0</kbd>(テンキー)</kbd>ミュート
    * <kbd>Home</kbd><kbd>End</kbd><kbd>←</kbd><kbd>→</kbd><kbd>↑</kbd><kbd>↓</kbd></kbd>はテンキーも同等

### 補足

AXキーボードの日本語入力関連キーの日本語入力向け機能はすべて別用途になっています。101キーボードでの日本語入力方法を使う必要があります([SKK日本語入力FEP](http://coexe.web.fc2.com/programs.html)を使うのも良いでしょう)。

<kbd><kbd>Fn</kbd>+<kbd>Pause</kbd></kbd>でスリープするのはこの変換器を作る前に使っていた[Razer Black Widow JP](https://www.razer.com/jp-jp/gaming-keyboards/razer-blackwidow-lite)(リンク先はUSキーボード)からのパクリ。

Windowsのキーボードドライバーを`kbdax2.dll`ではなく`kbd101.dll`にすると<kbd>漢字</kbd>は漢字キーではなく、単に<kbd>右Alt</kbd>として動作します(参考: [Windowsで右Altキーに［漢字］キーを割り当てる方法（AXキーボード設定を利用する方法）](https://atmarkit.itmedia.co.jp/ait/articles/0001/26/news001.html))

101キーボードドライバー使用時は<kbd><kbd>Shift</kbd>+<kbd>Caps Lock</kbd></kbd>はMS-IMEの切り替えだそうです。<kbd><kbd>Alt</kbd>+<kbd>Shift</kbd>+<kbd>Caps Lock</kbd></kbd>でCaps Lockとして動作するようです。

同じく101キーボードドライバー使用時は<kbd><kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Caps Lock</kbd></kbd>は**かなロック**として動作するようです。残念ながらWindows側から通知が来ないため、キーボードの**カナLock**ランプを点灯させるような動作はできませんでした。うっかり**かなロック**状態になって困った場合は再度<kbd><kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Caps Lock</kbd></kbd>で解除できます。

## 参考文献

* [Japanese Keyboard (layout and scancode)](http://hp.vector.co.jp/authors/VA003720/lpproj/others/kbdjpn.htm)
* [主要キーレイアウトのキーとその Scancode まとめ](https://bsakatu.net/doc/scancode/)
* [PS2 Commands](https://techdocs.altium.com/display/FPGA/PS2+Commands)
* [HID Usage Tables 1.4](https://usb.org/document-library/hid-usage-tables-14)
