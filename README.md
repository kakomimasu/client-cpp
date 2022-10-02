# 囲みマスC++クライアント

## 使い方

`main.cpp`にサンプルのコードが書かれています。
`main.cpp`は`sample/client_a1.cpp`の内容に加えオプション設定方法などが書かれています。
何も設定しない状態では、ゲストユーザとして参加します。

ゲスト名を変える場合やAI対戦をする場合などにはコメント部分を解除して使用してください。


## ビルド(Windows)

### Visual Studioでのビルドの手順

- Visual Studioをインストールします
  - https://visualstudio.microsoft.com/ja/
  - 「C++によるデスクトップ開発」がインストールされている必要があります。
- curlをインストールします
  - https://curl.se/windows/
- Visual Studioで「空のプロジェクト(C++)」を作成します
  - プロジェクトに次のファイルを追加します 「`main.cpp`、`kakomimasu.cpp`、`kakomimasu.h`、`picojson.h`」
  - メニューバーの「プロジェクト」→「○○のプロパティ」→「構成プロパティ」を選びます
    - 「全般」→「C++ 言語標準」→「ISO C++17 標準 (std:c++17)」を選択します

### GCC(Windows)でのビルドの手順

- MinGW-w64をインストールします
  - https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download
  - Settingsは以下のようにします
    - Version: 8.1.0
    - Architecture: i686
    - Threads: posix
    - Exception: dwarf
    - Build revision: 0
  - 以下にパスを通します
    - C:\Program Files (x86)\mingw-w64\i686-8.1.0-posix-dwarf-rt_v6-rev0\mingw32\bin
    - C:\Program Files (x86)\mingw-w64\i686-8.1.0-posix-dwarf-rt_v6-rev0\mingw32\i686-w64-mingw32\bin
- curlをインストールします
  - https://curl.se/windows/
- コンパイルします
  - g++ -std=c++17 main.cpp kakomimasu.cpp

コンパイルできない場合は、環境変数の設定でMinGWの優先度を一番上にするとうまくいく場合があります。
