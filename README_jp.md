<p align="right">
  <a href="./README.md">English</a>
</p>

# OpenOSD-X

**OpenOSD-X** は、アナログ OSD と VTX 機能を統合したプロジェクトです。

## 特徴

* デジタル専用 FC（アナログ OSD 非搭載）と組み合わせることで、5.8GHz アナログ映像を利用可能
* MAX7456 と同等解像度の **SD 版** に加え、より多くの情報を表示できる **HD 版** を用意
* Betaflight Configurator からフォントアップデートが可能
* 小型・低価格な **STM32G431KBT** を採用し、OSD 単体でも使用可能

### 開発中の様子

[![Watch the video](https://img.youtube.com/vi/iuA0HPM-mJo/0.jpg)](https://youtu.be/iuA0HPM-mJo)

---

## 接続例

![connection](doc/Connection.png)

---

## 設定方法

1. カメラ、OpenODS-X、FC、PC を接続する
2. すべての機器に電源を供給する。必要に応じてバッテリーも接続する。
3. Betaflight Configurator を起動する
4. シリアルポート設定

   * OpenOSD-X が接続されたポートを「VTX (MSP+DisplayPort)」に設定
   * 全機器を再起動すると、OSD・VTX のパラメータが読み出される
5. OSD 設定

   * 「HD」を選択（NTSC/PAL ではない）
   * 解像度はカメラやファームにより自動設定される
6. VTX 設定

   * VTX テーブルは内蔵されており、自動設定される
   * チャンネルと出力を設定する

---

## ファームウェア更新方法
FC経由(SerialPassthrough)またはUSB-Serial(FTDI)でファームウェアアップデートが可能です
1. OpenVTX 基板、FC、PC を接続する
2. Python スクリプトを実行する（自動で COM ポートを検出して書き込み）

   ```bash
   python flashOpenOSD-X.py hex-file
   ```

   ⚠ Betaflight のシリアルポート（DisplayPort-VTX）が有効になっている必要あり

---

## HEX ファイル

最新の HEX ファイルはこちら:
[https://github.com/OpenOSD-X/OpenOSD-X/actions](https://github.com/OpenOSD-X/OpenOSD-X/actions)

リリースは機種・解像度ごとに提供：

* `openosd-x_breakoutboard_sd.hex` …… breakoutboard 用 通常解像度版
* `openosd-x_breakoutboard_hd.hex` …… breakoutboard 用 高解像度版

---
## フォントアップデート

Betaflight Configurator からフォントアップデートが可能です

* Betaflight 2025.12.0以降が必要です。
* Betaflightのファームはカスタム定義"USE_MSP_DISPLAYPORT_FONT"でbuildしたファームを使用してください
* configratorもファームに対応したものを使用してください。


---

# ハード開発者向け情報

## リファレンス回路図・ブロック図

[https://github.com/OpenOSD-X/OpenOSD-X/tree/main/doc](https://github.com/OpenOSD-X/OpenOSD-X/tree/main/doc)

## 開発ボードでのテスト

ST 製 Nucleo や WeACT 製開発ボードなどで OSD 部分のみをテスト可能。
ターゲットは STM32G431KBT だが、STM32G431C や G473 でも動作確認できる。

## 初回書き込み

ST-LINK などで以下 2 つの HEX を書き込む：

* アプリケーションファーム: `openosd-x_***.hex`
* ブートローダ: `openosd-x_bootloader.hex`

## VPD テーブル

VTX の PA 設定に用いるテーブル。VTX を使わない場合は不要。
例（breakoutboard 用）：

| 　            | 5600MHz | 5650MHz | 5700MHz | 5750MHz | 5800MHz | 5850MHz | 5900MHz | 5950MHz | 6000MHz |
| ------------ | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- |
| 14dBm(25mW)  | 1300mV  | 1330mV  | 1345mV  | 1400mV  | 1480mV  | 1590mV  | 1670mV  | 1710mV  | 1760mV  |
| 20dBm(100mW) | 1910mV  | 1970mV  | 1980mV  | 2120mV  | 2270mV  | 2430mV  | 2540mV  | 2620mV  | 2750mV  |

独自基板を製造する場合は、PA や実装部品に合わせた VPD テーブルを作成する必要がある。
dev 版ファームを利用して測定・作成する。

---

## dev 版ファームの利用

TX 調整や VPD テーブル作成に使用する開発用ファーム。

### 接続方法

```text
[PC (Terminal)] --- UART (115200bps) --- [OpenOSD-X (dev版)] --- 5.8GHz --- [計測器]
```

### コマンド例

```text
> vtx_set 5800 1000    ...... 周波数 5800MHz, VPD ターゲット電圧 1000mV
```

目的の送信出力になるよう電圧を調整し、結果をまとめたものが VPD テーブルとなる。

---


# ファーム開発者向け情報

## build方法
cmakeを利用した一般的な手法でbuildします
- 事前にsrecordをインストールします
  ```
  $ sudo apt-get install -y srecord
  ```
- buildします
  ```
  $ mkdir -p build && cd build
  $ cmake -DTARGET=brakoutboard ..
  $ make
  ```
- cmakeのオプション例
  ```
  $ cmake -DTARGET=novtx    vtx機能なし
  $ cmake -DRESOLUTION_HD=ON    HD版 　　
  ```

##  flash memory map

| address           | note |
| ----------------- | --- |
| 0801F800-0801FFFF | 個別のVPDテーブル(オプション) |
| 0801B800-0801F7FF | フォント |
| 0801B000-0801B7FF | 設定保存 |
| 08004200-0801AFFF | OpenOSD-X application |
| 08004000-080041FF | OpenOSD-X application-header |
| 08000000-08003FFF | bootloader |

###  application-header
| address           | note |
| ----------------- | --- |
| 08004000-08004003 | applicationのサイズ |
| 08004004-08004007 | applicationのCRC(STM32) |
| 08004080-0800408F | ボード名称(文字列) |
| 08004090-0800409F | バージョン(文字列) |


## Developers Channel

専用 Discord サーバー:
[https://discord.gg/9qxJNTPSPs](https://discord.gg/9qxJNTPSPs)

## Open Source

OpenOSD-X はオープンソースソフトウェアであり、無保証で無償提供される。
ソースコードの一部は以下のプロジェクトを基にしている：

* betaflight
* OpenVTx
* OpenPixelOSD

