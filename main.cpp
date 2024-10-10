#include <bits/stdc++.h>
using namespace std;

using ll = long long;

#define CANT 0 // 進めない
#define CW 1 // 時計回り
#define CCW 2 // 反時計回り

// 回転壁の構造体
struct kaiten_kabe {
    ll x, y; // 回転壁の位置
    ll shape; // 回転壁の形状
    ll M; // 同期している回転壁の数
    vector<ll> D; // 同期している回転壁のインデックス
    kaiten_kabe(ll x, ll y, const string& shape_str, ll M, vector<ll> D) : x(x), y(y) , M(M), D(D) {
        shape = stoi(shape_str, nullptr, 8); // 文字列を8進数として整数に変換
    }
};

// 8方向の移動ベクトル
vector<ll> dx = {1, 1, 0, -1, -1, -1, 0, 1};
vector<ll> dy = {0, 1, 1, 1, 0, -1, -1, -1};
// 8の累乗
vector<ll> Pow8 = {1, 8, 64, 512, 4096, 32768, 262144, 2097152, 16777216};

ll H, W; // グリッドの高さと幅

// 座標が範囲内かどうかをチェックする関数
bool can_move(ll x, ll y, ll H, ll W) {
    if (0 <= x && x < H && 0 <= y && y < W) return true;
    return false;
}

// keyのk番目の状態を取得する関数
ll get_kth_state(ll key, ll k) {
    return (key / Pow8[k]) % 8;
}

// 回転可能かどうかを判定する関数
ll can_rotate(ll x, ll y, ll dir, vector<string> &grid, vector<kaiten_kabe> &kaiten) {
    ll nx = x + dx[dir];
    ll ny = y + dy[dir];
    ll k = grid[nx][ny] - '0'; // グリッド上の回転壁のインデックスを取得
    ll torque = dx[dir] * (y - kaiten[k].y) - dy[dir] * (x - kaiten[k].x); // トルクを計算

    if (torque == 0) return CANT; // トルクが0なら回転できない
    if (torque > 0) return CW; // トルクが正なら時計回り
    return CCW; // トルクが負なら反時計回り
}

// 次の状態のkeyを計算する関数
ll next_key(ll key, ll k, ll rot, vector<kaiten_kabe> &kaiten) {
    ll nkey = key;
    for (auto d : kaiten[k].D) {
        ll d_state = get_kth_state(key, d); // 同期している回転壁の状態を取得
        ll nd_state;
        if (rot == CW) {
            nd_state = (d_state + 7) % 8; // 時計回りに回転
        }
        else {
            nd_state = (d_state + 1) % 8; // 反時計回りに回転
        }
        nkey += (nd_state - d_state) * Pow8[d]; // 同期している回転壁を回転
    }
    return nkey; // 新しいkeyを返す
}

// 幅優先探索を行う関数
void bfs(ll sx, ll sy, ll gx, ll gy, map<ll, vector<string>> &grids, vector<kaiten_kabe> &kaiten, set<ll> &valid_states, map<ll, vector<vector<ll>>> &dist, map<ll, vector<vector<tuple<ll, ll, ll>>>> &par) {
    ll N = kaiten.size();
    queue<tuple<ll, ll, ll>> q; // キューを初期化
    q.push(make_tuple(sx, sy, 0)); // スタート地点をキューに追加
    dist[0][sx][sy] = 0; // スタート地点の距離を0に設定

    while (!q.empty()) {
        ll x, y, key;
        tie(x, y, key) = q.front(); // キューの先頭を取得
        q.pop(); // キューの先頭を削除

        if (x == gx && y == gy) { // ゴールに到達した場合
            return;
        }

        for (ll i = 0; i < 8; i += 2) { // 8方向に移動を試みる
            ll nkey = key;
            ll nx = x + dx[i];
            ll ny = y + dy[i];
            if (!can_move(nx, ny, H, W) || grids[key][nx][ny] == '#' || grids[key][nx][ny] == '*') continue; // 移動先が範囲外または壁の場合はスキップ
            if (grids[key][nx][ny] != '.') { // 移動先が回転壁の場合
                ll k = grids[key][nx][ny] - '0'; // 回転壁のインデックスを取得
                ll rot = can_rotate(x, y, i, grids[key], kaiten); // 回転可能かどうかを判定

                if (rot == CANT) continue; // 回転できない場合はスキップ

                nkey = next_key(key, k, rot, kaiten); // 次の状態のkeyを計算
                if (valid_states.find(nkey) == valid_states.end()) continue; // 無効なkeyの場合はスキップ
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
ll rotl8(ll x, ll shift) {
    ll max = Pow8[8];
    shift *= 3; // 8進数なので3ビットずつ回転
    return (x << shift | (x >> (24 - shift))) % max; // 左シフトと右シフトを組み合わせて回転
}

// 回転壁を配置する関数
void put_kaiten_kabe(ll key, map<ll, vector<string>> &grids, vector<kaiten_kabe> &kaiten, set<ll> &valid_states, vector<string> S) {
    ll N = kaiten.size();

    for (ll k = 0; k < N; k++) {
        ll state = get_kth_state(key, k); // 現在の状態を取得
        kaiten_kabe kk = kaiten[k];
        ll x = kk.x;
        ll y = kk.y;
        ll shape = rotl8(kk.shape, state); // 回転後の形状を計算

        if (S[x][y] != '.') { // 回転壁の位置に既に回転壁や壁がある場合
            return;
        }
        S[x][y] = '*'; // 回転壁の位置を設定
        for (ll i = 0; i < 8; i++) {
            ll cnt = shape / Pow8[i] % 8; // 回転壁の長さを取得
            for (ll j = 0; j < cnt; j++) {
                ll nx = x + dx[i] * (j + 1);
                ll ny = y + dy[i] * (j + 1);
                if (can_move(nx, ny, H, W) && S[nx][ny] == '.') {
                    S[nx][ny] = k + '0';
                }
                else {
                    return;
                }
            }
        }
    }
    valid_states.insert(key); // 有効な状態の集合に追加
    grids[key] = S; // グリッドを設定
}

// 全ての状態のグリッドを作成する関数
void make_grids(map<ll, vector<string>> &grids, vector<kaiten_kabe> &kaiten, set<ll> &valid_states, ll state_num, vector<string> &S) {
    for (ll key = 0; key < state_num; key++) {
        put_kaiten_kabe(key, grids, kaiten, valid_states, S); // 各状態に対して回転壁を配置
    }
}

// 結果を出力する関数
void cout_result(ll gx, ll gy, map<ll, vector<string>> &grids, map<ll, vector<vector<ll>>> &dist, map<ll, vector<vector<tuple<ll, ll, ll>>>> &par) {
    ll ans = -1;
    ll goal_key = -1;
    for (auto key : dist) {
        if (dist[key.first][gx][gy] != -1) { // ゴールに到達した場合
            ans = dist[key.first][gx][gy];
            goal_key = key.first;
            break;
        }
    }

    stack<tuple<ll, ll, ll>> path;
    ll key = goal_key;
    ll x = gx;
    ll y = gy;
    while (key != -1 && x != -1 && y != -1) { // 経路を逆順に辿る
        ll pkey, px, py;
        tie(pkey, px, py) = par[key][x][y];
        path.push(make_tuple(key, x, y));
        key = pkey;
        x = px;
        y = py;
    }

    cout << ans << endl; // 最短距離を出力
    while (!path.empty()) {
        cout << endl;
        ll key, x, y;
        tie(key, x, y) = path.top();
        path.pop();
        vector<string> grid = grids[key];
        grid[x][y] = 'o'; // 経路を'o'でマーク
        for (ll i = 0; i < H; i++) {
            cout << grid[i] << endl; // グリッドを出力
        }
    }
}

// メイン関数
int main() {
    cin >> H >> W; // グリッドの高さと幅を入力
    ll sx, sy, gx, gy;
    cin >> sx >> sy >> gx >> gy; // スタートとゴールの座標を入力
    vector<string> S(H);
    for (ll i = 0; i < H; i++) {
        cin >> S[i]; // グリッドの状態を入力
    }

    ll N;
    cin >> N; // 回転壁の数を入力
    vector<kaiten_kabe> kaiten;
    for (ll i = 0; i < N; i++) {
        ll x, y;
        string shape;
        cin >> x >> y >> shape; // 回転壁の位置と形状を入力
        ll M;
        cin >> M; // 同期している回転壁の数を入力
        vector<ll> D(M);
        for (ll j = 0; j < M; j++) {
            cin >> D[j]; // 同期している回転壁のインデックスを入力
        }
        kaiten.push_back(kaiten_kabe(x, y, shape, M, D)); // 回転壁を追加
    }

    ll state_num = Pow8[N]; // 状態の数を計算
    set<ll> valid_states; // 有効な状態の集合
    map<ll, vector<string>> grids; // 全ての状態のグリッドを初期化
    make_grids(grids, kaiten, valid_states, state_num, S); // 全ての状態のグリッドを作成
    map<ll, vector<vector<ll>>> dist; // 距離を初期化
    for (auto key : valid_states) {
        dist[key] = vector<vector<ll>>(H, vector<ll>(W, -1)); // 距離を初期化
    }
    
    map<ll, vector<vector<tuple<ll, ll, ll>>>> par; // 親ノードを初期化
    for (auto key : valid_states) {
        par[key] = vector<vector<tuple<ll, ll, ll>>>(H, vector<tuple<ll, ll, ll>>(W, make_tuple(-1, -1, -1))); // 親ノードを初期化
    }
    bfs(sx, sy, gx, gy, grids, kaiten, valid_states, dist, par); // 幅優先探索を実行

    cout_result(gx, gy, grids, dist, par); // 結果を出力
}