# ビルド方法

1. Xcode インストール(Macのみ)  
Xcode は AppStoreからインストールできます. インストール後 一度起動しておく.  
*Windows は Qt と一緒に MinGW をインストールする.  

1. Qt のインストール  
Qt のホームページに行き OpenSource 版のインストーラーをダウンロードして実行する.  
コンポーネントの選択でいかの項目にチェックを入れる
    - WIndowsの場合
        - 「Qt」 - 「Qt 5.12.0」 - 「MinGW 7.3.0 64-bit」
        - 「Qt」 - 「Developer and Designer Tools」 - 「MinGW 7.3.0 64-bit」

1. CMake のインストール

1. opencv のビルド
github の opencv/opencv のリポジトリから opencv-4.0.1 と opencv/opencv_contrib のリポジトリから opencv_contrib-4.0.1 のソースコードをダウンロードし,
適当なフォルダに回答しておく
    - Windows
        1. スタートメニューから「Qt 5.12.0 for Desktop (MinGW 7.3.0 64-bit)」を選んでプロンプトを開く
        1. プロンプトから 「C:\Program Files\CMake\bin\cmake-gui.exe」 を起動する 
        1. Where is the source code に Opencv を解凍したフォルダを指定する
        1. Where to build the binaries に build フォルダを指定する
        1. BUILD_opencv_world にチェックをいれる
        1. OPENCV_EXTRA_MODULES_PATH の項目に <opencv_contrib>/modules をいれる
        1. BUILD_opencv_cvv のチェックを外す
        1. WITH_MSMF のチェックを外す
        1. WITH_VTK のチェックを外す
        1. Configure ボタンを押す
        1. もう一度 Configure ボタンを押す
        1. Generate ボタンを押す
        1. コマンドプロンプトに戻り 4. で指定した build フォルダに移動し 「mingw32-make install」 と入力  
        コンパイルが終わるまで気長に待つ
        
1. src/RacingViewer.pro をダブルクリックして Qt Creator を立ち上げる  
Configure Project を押す

1. build
