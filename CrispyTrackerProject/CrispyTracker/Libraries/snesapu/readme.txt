-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  改良版 SNESAPU.DLL (他プレイヤー用)  v2.20.1
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                               Copyright (C) 2003-2023 デグレファクトリー

■ 目次

  1. 概要
  2. インストール方法
  3. プレイヤー開発者の方向け
  4. ライブラリについて
  5. ライセンス


□ 概要
========================================

  "改良版 SNESAPU.DLL" をダウンロードしていただきまして、ありがとうございます。

  "改良版 SNESAPU.DLL" は再現性・音質に優れた Alpha-II Productions 製 SNESAPU ライブラリ
  v2.0 をベースに独自に改良を加え、高機能・軽量・安定化した SHVC-SOUND エミュレータです。


  改良版 SNESAPU.DLL の特徴は以下のとおりです。

    ・ DSP の内部処理で、32bit-float (x87 FPU) により演算を行うことで、実機よりも高音質の
       出力結果が得られます。 実機再現性と音質向上の両立を目指しています。

    ・ 保存しただけでは演奏できない特殊な SPC を演奏させる拡張規格 Script700 (SE) の
       互換機能を搭載しています (SNESAPU.DLL 単体では TimerTrick 互換機能もあります)。

    ・ Script700 ファイルを自動的に読み込み、解析・実行するための API が用意されている
       ため、比較的簡単に Script700 を使用した演奏プログラムを実装できます。

    ・ シリアルポートや G.I.M.I.C 経由による、実機のサウンドチップを使った演奏を行う
       ための通信 API が実装されているため、簡単に SHVC-SOUND の制御が行えます。


  本ライブラリは以下のオペレーティングシステム上で動作するよう設計されています。

    ・ 32bit 版 Windows 2000, XP, Vista, 7, 8, 8.1, 10
    ・ 64bit 版 Windows Vista, 7, 8, 8.1, 10, 11 (※)

    ※ 64bit 環境であっても 32bit 版のソフトウェア上でのみ動作します。


□ インストール方法
========================================

  この SNESAPU.DLL は "SNES SPC700 Player" 以外のプレイヤー用に設計されております。
                                           ￣￣
  詳細なインストール方法や、対応プレイヤーの最新情報は "SNES SPC700 Player" ダウンロード
  ページの『改良版 SNESAPU.DLL 対応プラグイン』をご覧ください。

    ⇒ https://dgrfactory.jp/spcplay/plugin.html


□ プレイヤー開発者の方向け
========================================

  この SNESAPU.DLL で使用可能な API の一覧は、"SNES SPC700 Player" ダウンロードページの
  『改良版 SNESAPU.DLL の概要』をご覧ください。

    ⇒ https://dgrfactory.jp/spcplay/snesapu.html


□ ライブラリについて
========================================

  改良版 SNESAPU.DLL に関するサポートはすべて『デグレファクトリー』(degrade-factory) で
  行っています。

  確認されている制限事項、既知の不具合情報は、『デグレファクトリー』の "GitHub" ページに
  最新の情報が記載されていますので、あわせてご参照ください。

    ⇒ https://github.com/dgrfactory/spcplay/issues


  このソフトウェアは、主に以下のファイル、または資料を使用して開発されています。
  貴重な資料を公開していただき、ありがとうございます。

  ・ Alpha-II Productions       - SNESAPU v0.99, v1.0.1, v2.0, v3.0, v3.0a
  ・ Alpha-II Productions       - SPC File Format
  ・ Alpha-II Productions       - APU Manual
  ・ ekid 氏                    - SNES SPCTECH SPC-700 Reference
  ・ Butcha 氏                  - The Bit Rate Reduction sound encoding scheme
  ・ gochaism 氏, Snes9x team   - Snes9x 1.51 re-recording (Snes9x-rr) v5.2
  ・ Snes9x team                - Snes9x 1.60
  ・ sanmaiwashi 氏             - uosnes 20100825
  ・ byuu 氏                    - bsnes v115
  ・ Raphael Assenat 氏         - vspcplay 1.3
  ・ SNES Wiki


  このソフトウェアは、主に以下の規格、または互換規格が使用されています。
  便利な規格を提唱していただき、ありがとうございます。

  ・ 41568k 氏                  - Script700
  ・ 木下氏                     - Script700 SE
  ・ 木下氏・黒羽氏・PEN@海猫氏 - TimerTrick
  ・ Nitro 氏                   - ID666
  ・ zsKnight氏・_Demo_氏       - SPC700 File Format


  ソフトウェアの改良にあたって、下記の方に特にお世話になりました (Special Thanks!)。
  いつも、ありがとうございます。

  ・ 黒羽氏                     - 改良版 SNESAPU.DLL の詳細な不具合情報・ご意見のご提供
  ・ あすか氏                   - 改良版 SNESAPU.DLL の詳細な不具合情報・ご意見のご提供
  ・ fastlast 氏                - 改良版 SNESAPU.DLL の詳細な不具合情報・ご意見のご提供
  ・ Gnilda 氏                  - 改良版 SNESAPU.DLL の詳細な不具合情報・ご意見のご提供
  ・ KungFuFurby 氏             - 改良版 SNESAPU.DLL の詳細な不具合情報・ご意見のご提供
  ・ 掲示板・メールでご意見・ご感想を送っていただいたご利用者の皆さま
  ・ GitHub で issue 報告をいただいたご利用者の皆さま


  開発で使わせていただきましたソフトウェア、ソースコードの作者、各規格の立案者、そして
  本ソフトウェアをお使いいただき、さらにはご感想、ご意見、ご要望までくださった方々には、
  たいへん感謝いたします。 いつも、ありがとうございます。


  SNESAPU.DLL : Copyright (C) 1999-2008 Alpha-II Productions,
                          (C) 2003-2023 degrade-factory.

  デグレファクトリー   : https://dgrfactory.jp
    -> GitHub          : https://github.com/dgrfactory/spcplay
  Alpha-II Productions : https://www.alpha-ii.com

  ※ SNES および Super Nintendo Entertainment System は、米国 Nintendo Co. の商標です。


□ ライセンス
========================================

  *** GNU General Public License v2.0 ***

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
