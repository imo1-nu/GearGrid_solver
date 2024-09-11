#include <bits/stdc++.h>
using namespace std;

#define CANT 0 // 進めない
#define CW 1 // 時計回り
#define CCW 2 // 反時計回り

// 回転扉の構造体
struct kaiten_tobira {
    int x, y, shape;
    kaiten_tobira(int x, int y, const string& shape_str) : x(x), y(y) {
        shape = stoi(shape_str, nullptr, 8); // 文字列をビット列として整数に変換
    }
};

// 8方向の移動ベクトル
vector<int> dx = {1, 1, 0, -1, -1, -1, 0, 1};
vector<int> dy = {0, 1, 1, 1, 0, -1, -1, -1};
// 8の累乗
vector<int> Pow8 = {1, 8, 64, 512, 4096, 32768, 262144, 2097152, 16777216};

// 座標が範囲内かどうかをチェックする関数
bool can_move(int x, int y, int H, int W) {
    if (0 <= x && x < H && 0 <= y && y < W) return true;
    return false;
}

// keyのk番目の状態を取得する関数
int get_kth_state(int key, int k, int N) {
    return (key / Pow8[k]) % 8;
}

// 回転可能かどうかを判定する関数
int can_rotate(int x, int y, int dir, vector<string> &grid, vector<kaiten_tobira> &kaiten) {
    int nx = x + dx[dir];
    int ny = y + dy[dir];
    int k = grid[nx][ny] - '0'; // グリッド上の回転扉のインデックスを取得
    int torque = dx[dir] * (y - kaiten[k].y) - dy[dir] * (x - kaiten[k].x); // トルクを計算

    if (torque == 0) return CANT; // トルクが0なら回転できない
    if (torque > 0) return CW; // トルクが正なら時計回り
    return CCW; // トルクが負なら反時計回り
}

// 次の状態のkeyを計算する関数
int next_key(int key, int k, int rot, int N) {
    int state = get_kth_state(key, k, N); // 現在の状態を取得
    int next_state;
    if (rot == CW) {
        next_state = (state + 7) % 8; // 時計回りに回転
    }
    else {
        next_state = (state + 1) % 8; // 反時計回りに回転
    }
    return key + (next_state - state) * Pow8[k]; // 新しいkeyを計算
}

// 幅優先探索を行う関数
void bfs(int sx, int sy, int gx, int gy, vector<vector<string>> &grids, vector<kaiten_tobira> &kaiten, set<int> &unable_key, vector<vector<vector<int>>> &dist, vector<vector<vector<tuple<int, int, int>>>> &par) {
    int H = grids[0].size();
    int W = grids[0][0].size();
    int N = kaiten.size();
    queue<tuple<int, int, int>> q; // キューを初期化
    q.push(make_tuple(sx, sy, 0)); // スタート地点をキューに追加
    dist[0][sx][sy] = 0; // スタート地点の距離を0に設定

    while (!q.empty()) {
        int x, y, key;
        tie(x, y, key) = q.front(); // キューの先頭を取得
        q.pop(); // キューの先頭を削除

        if (x == gx && y == gy) { // ゴールに到達した場合
            return;
        }

        for (int i = 0; i < 8; i += 2) { // 8方向に移動を試みる
            int nkey = key;
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (!can_move(nx, ny, H, W) || grids[key][nx][ny] == '#' || grids[key][nx][ny] == '*') continue; // 移動先が範囲外または壁の場合はスキップ
            if (grids[key][nx][ny] != '.') { // 移動先が回転扉の場合
                int k = grids[key][nx][ny] - '0'; // 回転扉のインデックスを取得
                int rot = can_rotate(x, y, i, grids[key], kaiten); // 回転可能かどうかを判定

                if (rot == CANT) continue; // 回転できない場合はスキップ

                nkey = next_key(key, k, rot, N); // 次の状態のkeyを計算
                if (unable_key.count(nkey)) continue; // 無効なkeyの場合はスキップ
            }
            
            if (dist[nkey][nx][ny] == -1) { // 移動先が空白の場合
                dist[nkey][nx][ny] = dist[key][x][y] + 1; // 距離を更新
                par[nkey][nx][ny] = make_tuple(key, x, y); // 親ノードを設定
                q.push(make_tuple(nx, ny, nkey)); // キューに追加
            }
        }
    }
}

// 8桁8進数の回転を行う関数
int rotl8(int x, int shift) {
    int max = Pow8[8];
    return ((x << (shift) * 3) | (x >> ((8 - shift) * 3))) % max; // 左シフトと右シフトを組み合わせて回転
}

// 回転扉を配置する関数
void put_kaiten_tobira(int key, vector<string> &grid, vector<kaiten_tobira> &kaiten, set<int> &unable_key) {
    int H = grid.size();
    int W = grid[0].size();
    int N = kaiten.size();

    for (int k = 0; k < N; k++) {
        int state = get_kth_state(key, k, N); // 現在の状態を取得
        kaiten_tobira kt = kaiten[k];
        int x = kt.x;
        int y = kt.y;
        int shape = rotl8(kt.shape, state); // 回転後の形状を計算

        if (grid[x][y] != '.') { // 回転扉の位置に既に回転扉や壁がある場合
            unable_key.insert(key); // 無効なkeyを記録
            return;
        }
        grid[x][y] = '*'; // 回転扉の位置を設定
        for (int i = 0; i < 8; i++) {
            int cnt = shape / Pow8[i] % 8; // 回転扉の長さを取得
            for (int j = 0; j < cnt; j++) {
                int nx = x + dx[i] * (j + 1);
                int ny = y + dy[i] * (j + 1);
                if (can_move(nx, ny, H, W) && grid[nx][ny] == '.') {
                    grid[nx][ny] = k + '0';
                }
                else {
                    unable_key.insert(key); // 無効なkeyを記録
                    return;
                }
            }
        }
    }
}

// 全ての状態のグリッドを作成する関数
void make_grids(vector<vector<string>> &grids, vector<kaiten_tobira> &kaiten, set<int> &unable_key) {
    int state_num = grids.size();
    for (int key = 0; key < state_num; key++) {
        put_kaiten_tobira(key, grids[key], kaiten, unable_key); // 各状態に対して回転扉を配置
    }
}

// 結果を出力する関数
void cout_result(int gx, int gy, vector<vector<string>> &grids, vector<vector<vector<int>>> &dist, vector<vector<vector<tuple<int, int, int>>>> &par) {
    int ans = -1;
    int goal_key = -1;
    int state_num = grids.size();
    int H = grids[0].size();
    for (int key = 0; key < state_num; key++) {
        if (dist[key][gx][gy] != -1) { // ゴールに到達した場合
            ans = dist[key][gx][gy];
            goal_key = key;
            break;
        }
    }

    stack<tuple<int, int, int>> path;
    int key = goal_key;
    int x = gx;
    int y = gy;
    while (key != -1 && x != -1 && y != -1) { // 経路を逆順に辿る
        int pkey, px, py;
        tie(pkey, px, py) = par[key][x][y];
        path.push(make_tuple(key, x, y));
        key = pkey;
        x = px;
        y = py;
    }

    cout << ans << endl; // 最短距離を出力
    while (!path.empty()) {
        cout << endl;
        int key, x, y;
        tie(key, x, y) = path.top();
        path.pop();
        vector<string> grid = grids[key];
        grid[x][y] = 'o'; // 経路を'o'でマーク
        for (int i = 0; i < H; i++) {
            cout << grid[i] << endl; // グリッドを出力
        }
    }
}

// メイン関数
int main() {
    int H, W;
    cin >> H >> W; // グリッドの高さと幅を入力
    int sx, sy, gx, gy;
    cin >> sx >> sy >> gx >> gy; // スタートとゴールの座標を入力
    vector<string> S(H);
    for (int i = 0; i < H; i++) {
        cin >> S[i]; // グリッドの状態を入力
    }

    int N;
    cin >> N; // 回転扉の数を入力
    vector<kaiten_tobira> kaiten;
    for (int i = 0; i < N; i++) {
        int x, y;
        string shape;
        cin >> x >> y >> shape; // 回転扉の位置と形状を入力
        kaiten.push_back(kaiten_tobira(x, y, shape));
    }

    int state_num = Pow8[N]; // 状態の数を計算
    set<int> unable_key; // 無効なkeyを記録するセット
    vector<vector<string>> grids(state_num, S); // 全ての状態のグリッドを初期化
    make_grids(grids, kaiten, unable_key); // 全ての状態のグリッドを作成
    vector<vector<vector<int>>> dist(state_num, vector<vector<int>>(H, vector<int>(W, -1))); // 距離を初期化
    vector<vector<vector<tuple<int, int, int>>>> par(state_num, vector<vector<tuple<int, int, int>>>(H, vector<tuple<int, int, int>>(W, make_tuple(-1, -1, -1)))); // 親ノードを初期化
    bfs(sx, sy, gx, gy, grids, kaiten, unable_key, dist, par); // 幅優先探索を実行

    cout_result(gx, gy, grids, dist, par); // 結果を出力
}