#include <iostream>
#include <cstdio>
#include <string.h>
#include <random>
#include <cstdlib>
#include <ctime>
#include <set>
#include <algorithm>

#define GRID_SIZE 15            // 棋盘的规格
#define GRID_BLANK 0            // 空白
#define GRID_SELF 1             // 己方为1
#define GRID_OPPO -1            // 对方为-1
#define DISTANCE 1              // 周围的距离
#define SURROUND_MARK 2         // 用于标记棋子周围的棋格
#define N_TO_WIN 6              // 需要连续几子才能胜利
#define TIME_TO_SIMULATE 970    // 用于模拟的毫秒时间
#define INF 0x3f3f3f3f

using namespace std;
const int mov[8][2] = { {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1} };
                        // 零 一 二  三   四    五    六路
const int SelfValue[7] = { 1, 1, 20, 40,  200,  200,  1000000 };
const int OppoValue[7] = { 1, 1, 25, 50,  6000, 6000, 1000000 };

class Board
{
public:
    // 初始棋盘
    Board()
    {
        reset();
    }

    // 继承棋盘
    Board(Board* _board, set<int>* _legal_actions)
    {
        reset();
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                map[i][j] = _board->map[i][j];
            }
        }

        legal_actions = _legal_actions;
    }

    // 棋盘初始化
    void reset()
    {
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                map[i][j] = 0;
            }
        }
    }

    // 棋盘上落子
    void make_move(int move, int color)
    {
        int x = move / GRID_SIZE, y = move % GRID_SIZE;
        map[x][y] = color;
        change_mark(x, y);
    }

    // 标记周围待拓展的棋格
    void change_mark(int x, int y)
    {
        for (int i = -DISTANCE; i <= DISTANCE; i++)
        {
            for (int j = -DISTANCE; j <= DISTANCE; j++)
            {
                int ex = x + i, ey = y + j;
                if (ex >= 0 && ex < GRID_SIZE && ey >= 0 && ey < GRID_SIZE && map[ex][ey] == GRID_BLANK)
                    map[ex][ey] = SURROUND_MARK;
            }
        }
    }
    // 获取该棋盘的合法落子集合
    set<int>* get_legal_actions()
    {
        if (legal_actions != NULL)
            return legal_actions;     // 若已记录过集合，则直接返回集合
        legal_actions = new set<int>; // 反之，重新记录

        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                if (map[i][j] == SURROUND_MARK)
                    legal_actions->insert(i * GRID_SIZE + j);
            }
        }

        return legal_actions;
    }

public:
    int map[GRID_SIZE][GRID_SIZE]; // 15 * 15的棋盘
    set<int>* legal_actions;       // 合法落子的集合
};

//表示以（x,y）为源点，向direction方向延申 6-1格 的一条路
class Road {
public:
    int road_x, road_y;
    int direction;
    Road(int x_, int y_, int dir) :road_x(x_), road_y(y_), direction(dir) {}
public:

    //判断格子是否在棋盘中
    bool is_inGrid(int ex, int ey)const
    {
        return (ex >= 0 && ey >= 0 && ex < GRID_SIZE&& ey < GRID_SIZE);
    }

    //判断该路是否存在
    bool is_legalRoad() const
    {
        int max_dist = N_TO_WIN;
        if (!is_inGrid(road_x, road_y) || !is_inGrid(road_x + (max_dist - 1) * mov[direction][0], road_y + (max_dist - 1) * mov[direction][1]))//此路不存在
            return false;
        return true;
    }

    //计算以（x,y）为源点，向direction方向延申 6-1格 的路 的value
    pair<double, double> get_Road_Value(Board* state) const
    {
        int x = road_x, y = road_y;
        int max_dist = N_TO_WIN;
        if (!is_legalRoad())//此路不存在
            return { 0,0 };
        int self_cnt = 0, oppo_cnt = 0;
        for (int i = 0; i < max_dist; i++) {
            x += mov[direction][0];
            y += mov[direction][1];

            if (state->map[x][y] == GRID_SELF) self_cnt++;
            else if (state->map[x][y] == GRID_OPPO) oppo_cnt++;

            if (self_cnt > 0 && oppo_cnt > 0)return { 0,0 };//此路含有双方棋子，已无价值
        }
        return{ SelfValue[self_cnt], OppoValue[oppo_cnt] };
    }

    //提供set排列标准，随意设置，set主要用于去重
    bool operator< (const Road& t) const
    {
        if (road_x != t.road_x) return road_x > t.road_x;
        if (road_y != t.road_y) return road_y > t.road_y;
        return direction > t.direction;
    }
};

class TreeNode
{
public:
    // 树节点初始化
    TreeNode(TreeNode* _parent, Board* _state, int _color, int _action, int _depth, int _chesses)
    {
        parent = _parent;
        children.resize(0);
        state = _state;
        color = _color;
        reward = 0;
        visits = 0;
        action = _action;
        depth = _depth;
        max_depth = _depth;
        chesses = _chesses;
        if (is_root())
            set_allRoad_value();//对根节点的路全盘扫描 or 设一相对值
        else {
            diff_value = get_diff_road_StateValue(action);
            road_value = _parent->road_value + diff_value;
        }
    }

    // 判断节点是否被拓展满
    bool is_full_expand()
    {
        set<int>* legal_actions = state->get_legal_actions();
        return (int)legal_actions->size() <= (int)children.size();
    }

    // 判断节点是否叶节点
    bool is_leaf() { return children.size() == 0; }

    // 判断节点是否根节点
    bool is_root() { return parent == NULL; }

    //全盘扫描 or 设一相对值
    void set_allRoad_value() {//将根节点设为0，采用相对计算方式
        road_value = 0;
    }

    //基于路计算此状态棋盘棋形的value与父节点差值
    double  get_diff_road_StateValue(int act) {
        double pre_value = 0, now_value = 0;
        int x = act / GRID_SIZE, y = act % GRID_SIZE;

        //先去除该落子
        state->map[x][y] = GRID_BLANK;
        pre_value += update_Road_StateValue(x, y);

        //恢复action_1 action_2
        state->map[x][y] = color;
        now_value += update_Road_StateValue(x, y);
        //返回差值
        return now_value - pre_value;
    }

    //局部扫描 计算局部的价值
    double update_Road_StateValue(int x, int y) {
        int max_dist = N_TO_WIN;
        double self_total_value = 0, oppo_total_threat = 0;
        set<Road> road_changed; // set去掉重复的路

        pair<double, double> road_valu;
        for (int i = 0; i < 4; i++) { // 方向
            for (int dist = 0; dist < max_dist; dist++) { // 距离
                road_changed.insert({ x + dist * mov[i][0], y + dist * mov[i][1], i + 4 }); // i+4使路的方向与寻找路的源点的方向反向
            }
        }
        for (auto& road : road_changed) {
            road_valu = road.get_Road_Value(state);
            self_total_value += road_valu.first;
            oppo_total_threat += road_valu.second;
        }
        return  self_total_value - oppo_total_threat;
    }

public:
    TreeNode* parent;            // 父节点
    vector<TreeNode*> children;  // 儿子节点数组
    Board* state;                // 节点棋盘状态
    int color;                   // 节点扮演的角色
    int reward;                  // 节点的奖励点
    int visits;                  // 节点被访问次数
    int action;                  // 节点上发生的落子行为
    double road_value;           // 该节点的基于路分析的value 
    double diff_value;           // 从该节点落子的价值
    int depth;                   // 节点深度（用于调试）
    int max_depth;               // 已该根为子树的最大深度（用于调试）
    int chesses;                 // 能落子数
};

class Game
{
public:
    // 游戏初始化
    Game()
    {
        cur_Board = new Board(); // 棋盘初始化
        cur_player = GRID_SELF;  // 默认己方先手
        chesses = 1;             // 开始只能下一子
    }

    // 继承游戏（用于模拟）
    Game(Board* state, int color, int _chesses)
    {
        cur_Board = new Board(state, NULL); // 棋盘继承
        cur_player = color;                 // 颜色继承
        chesses = _chesses;                 // 余棋继承
    }

    // 合法落子
    bool legal_move(int move)
    {
        int x = move / GRID_SIZE, y = move % GRID_SIZE;
        if (x < 0 || y < 0 || x >= GRID_SIZE || y >= GRID_SIZE)
            return false;
        if (cur_Board->map[x][y] != 0)
            return false;
    }

    // 判断对局是否结束
    int is_end(Board* state)
    { // Return the winner of game
        // 横排六子
        for (int i = 0; i < GRID_SIZE; i++)
        {
            int count = 0;
            int prev_color = 0;
            for (int j = 0; j < GRID_SIZE; j++)
            {
                // cout << "[" << i << "," << j << "] ";
                int cur_color = state->map[i][j];
                if (cur_color != 0 && cur_color == prev_color)
                {
                    count++;
                    if (count >= N_TO_WIN)
                        return cur_color;
                }
                else
                {
                    count = 1;
                    prev_color = cur_color;
                }
            }
            // cout << endl;
        }

        // 竖排六子
        for (int j = 0; j < GRID_SIZE; j++)
        {
            int count = 0;
            int prev_color = 0;
            for (int i = 0; i < GRID_SIZE; i++)
            {
                // cout << "[" << i << "," << j << "] ";
                int cur_color = state->map[i][j];
                if (cur_color != 0 && cur_color == prev_color)
                {
                    count++;
                    if (count >= N_TO_WIN)
                        return cur_color;
                }
                else
                {
                    count = 1;
                    prev_color = cur_color;
                }
            }
            // cout << endl;
        }

        // 右斜六子上 左斜六子下
        for (int layer = 0; layer < GRID_SIZE; layer++)
        {
            int count = 0;
            int prev_color = 0;
            for (int k = 0; k < layer + 1; k++)
            {
                cout << "[" << layer - k << "," << k << "] ";
                int cur_color = state->map[layer - k][k];
                if (cur_color != 0 && cur_color == prev_color)
                {
                    count++;
                    if (count >= N_TO_WIN)
                        return cur_color;
                }
                else
                {
                    count = 1;
                    prev_color = cur_color;
                }
            }
            // cout << "*" << endl;
            for (int k = 0; k < GRID_SIZE - layer; k++)
            {
                // cout << "[" << layer + k << "," << k << "] ";
                int cur_color = state->map[layer + k][k];
                if (cur_color != 0 && cur_color == prev_color)
                {
                    count++;
                    if (count >= N_TO_WIN)
                        return cur_color;
                }
                else
                {
                    count = 1;
                    prev_color = cur_color;
                }
            }
            // cout << endl;
        }

        // 右斜六子下 左斜六子上
        for (int layer = 1; layer < GRID_SIZE; layer++)
        {
            int count = 0;
            int prev_color = 0;
            for (int k = 0; k < GRID_SIZE - layer; k++)
            {
                // cout << "[" << GRID_SIZE - k - 1 << "," << layer + k << "] ";
                int cur_color = state->map[GRID_SIZE - k - 1][layer + k];
                if (cur_color != 0 && cur_color == prev_color)
                {
                    count++;
                    if (count >= N_TO_WIN)
                        return cur_color;
                }
                else
                {
                    count = 1;
                    prev_color = cur_color;
                }
            }
            // cout << "&" << endl;
            for (int k = 0; k < GRID_SIZE - layer; k++)
            { // k: [7 -> 1]
                int cur_color = state->map[k][layer + k];
                // cout << "[" << k << "," << layer + k << "] ";
                if (cur_color != 0 && cur_color == prev_color)
                {
                    count++;
                    if (count >= N_TO_WIN)
                        return cur_color;
                }
                else
                {
                    count = 1;
                    prev_color = cur_color;
                }
            }
            // cout << endl;
        }

        return GRID_BLANK;
    }

    // 根据一步棋判断对局是否结束
    int is_end(Board* state, int move)
    { // Return the winner of game (by last step)
        int x = move / GRID_SIZE, y = move % GRID_SIZE;
        //判断color是否为surround_mark
        //if(state->map[x][y]==SURROUND_MARK)return 0;
        int cur_color = state->map[x][y];
        int count, i, j;

        // 竖排六子
        i = x;
        count = -1;
        while (i >= 0 && state->map[i][y] == cur_color)
        {
            i--;
            count++;
        }
        i = x;
        while (i < GRID_SIZE && state->map[i][y] == cur_color)
        {
            i++;
            count++;
        }
        if (count >= N_TO_WIN)
            return cur_color;

        // 横排六子
        j = y;
        count = -1;
        while (j >= 0 && state->map[x][j] == cur_color)
        {
            j--;
            count++;
        }
        j = y;
        while (j <= GRID_SIZE && state->map[x][j] == cur_color)
        {
            j++;
            count++;
        }
        if (count >= N_TO_WIN)
            return cur_color;

        // 右斜六子
        i = x;
        j = y;
        count = -1;
        while (i >= 0 && j < GRID_SIZE && state->map[i][j] == cur_color)
        {
            i--;
            j++;
            count++;
        }
        i = x;
        j = y;
        while (i < GRID_SIZE && j > 0 && state->map[i][j] == cur_color)
        {
            i++;
            j--;
            count++;
        }
        if (count >= N_TO_WIN)
            return cur_color;

        // 左斜六子
        i = x;
        j = y;
        count = -1;
        while (i >= 0 && j >= 0 && state->map[i][j] == cur_color)
        {
            i--;
            j--;
            count++;
        }
        i = x;
        j = y;
        while (i < GRID_SIZE && j < GRID_SIZE && state->map[i][j] == cur_color)
        {
            i++;
            j++;
            count++;
        }
        if (count >= N_TO_WIN)
            return cur_color;

        return GRID_BLANK;
    }

    // move落子
    void make_move(int move)
    {
        if (move == -1 || move  == -1 * GRID_SIZE - 1)
            return; // 不落子判断
        cur_Board->make_move(move, cur_player);
        chesses--;
        if (chesses == 0)
            change_turn();
    }

    // x y坐标落子
    void make_move(int x, int y)
    {
        if (x == -1 && y == -1)
            return; // 不落子判断
        int move = x * GRID_SIZE + y;
        cur_Board->make_move(move, cur_player);
        chesses--;
        if (chesses == 0)
            change_turn();
    }

    // 更换执棋者
    void change_turn()
    {
        chesses = 2;
        cur_player = -cur_player;
    }

public:
    Board* cur_Board; // 游戏此刻的棋盘
    int cur_player;   // 执棋者 己方 or 对方
    int chesses;      // 剩余落子数
};

class AIPlayer
{
public:
    // AI棋手初始化
    AIPlayer(Game* _game, int last_action)
    {
        game = _game;               // AI看到了游戏
        root = new TreeNode(NULL, _game->cur_Board, GRID_OPPO, last_action, 0, 1); // 心中建立起蒙特卡洛树根
        max_times = 6500;           // 最大模拟次数
        player_color = GRID_SELF;   // AI的棋色一定为己方
        c = 2;                      // 设置UCB公式的超参数
    }

    // 蒙特卡洛树搜索，返回应下的两步棋
    pair<int, int> mcts()
    {
        int reward = 0, real_times = 0;
        TreeNode* leave_node;
        clock_t start = clock();   // 起始时间
        for (int t = 0; t < max_times; t++)
        {
            leave_node = select_expand_node(root);                         // 选择要拓展的节点（初始是根节点）
            reward = simulate(leave_node);                                 // 从该节点模拟结果，返回奖励点
            back_propagate(leave_node, reward, leave_node->diff_value);    // 将奖励点反向传播
            clock_t end = clock(); // 结束时间
            //if (end - start >= TIME_TO_SIMULATE){real_times = t;break;}
        }
        //cout << root->max_depth << ' ' << real_times << endl;

        TreeNode* best_child_1 = select(root);                             // 从树中选择最佳子节点（最佳选择）
        TreeNode* best_child_2 = select(best_child_1);
        return { best_child_1->action, best_child_2->action };   // 返回最佳选择下的两步棋
    }

    // 从该节点选择子节点拓展
    TreeNode* select_expand_node(TreeNode* node)
    {
        while (game->is_end(node->state, node->action) == 0)
        { // 若棋局已结束
            if (node->children.size() == 0 || !node->is_full_expand())
            {                        // 若该节点无子节点 or 子节点未完全拓展
                return expand(node); // 拓展该节点
            }
            else
            {
                node = select(node); // 选择最佳子节点
            }
        }
        return node; // 返回结果
    }

    // 从该节点选择最佳子节点
    TreeNode* select(TreeNode* node)
    {
        double max_ucb = -INF;              // 最大UCB
        vector<TreeNode*> select_children; // 最佳子节点的集合

        for (auto child : node->children)
        {
            if (child->visits == 0)
            { // 未被访问的子节点直接选中
                return child;
            }
            // UCB计算公式
            double ucb = ((double)child->reward) / (double)child->visits + c * sqrt(2.0 * log((double)node->visits) / (double)child->visits) + child->diff_value / 20;
            if (ucb == max_ucb)
            { // 该节点的UCB和最大UCB一致，则加入集合
                select_children.push_back(child);
            }
            else if (ucb > max_ucb)
            { // 该节点的UCB更大，更新最大UCB，清空select_children
                select_children = { child };
                max_ucb = ucb;
            }
        }

        int n = (int)select_children.size();
        if (n == 0)
            return node->parent; // 若无最佳节点，则将父节点返回

        return select_children[rand() % n]; // 从最佳子节点集合随机选择一个
    }

    // 拓展选择的节点，返回一个可行的且未拓展过的子节点
    TreeNode* expand(TreeNode* node)
    {//
        set<int>* node_legal_actions = node->state->get_legal_actions(); // 该节点的合法落子集合

        set<int>* legal_actions = new set<int>;
        for (auto tmp : *node_legal_actions)
        {
            legal_actions->insert(tmp); // tmp为int i*GRID_SIZE+j
        }

        if (legal_actions->empty())                      
            return node->parent; // 若无处落子则返回父节点

        // 排除子节点的落子
        for (auto child : node->children)
        {
            legal_actions->erase(child->action);
        }

        set<int>::const_iterator it(legal_actions->begin()); // 集合指针it
        advance(it, rand() % legal_actions->size());         // 从余下的集合随机选择一步
        int action = *it;
        legal_actions->erase(action);                        // 用于获取action

        // 恢复
        for (auto child : node->children) 
        { 
            legal_actions->insert(child->action);
        }

        int new_node_color = node->color;
        int new_node_chesses = node->chesses - 1;
        if (new_node_chesses == 0) {
            new_node_color = -new_node_color;
            new_node_chesses = 2;
        }

        // 新节点的state
        Board* new_state = new Board(node->state, legal_actions);
        new_state->make_move(action, new_node_color);

        // 新节点
        TreeNode* new_node = new TreeNode(node, new_state, new_node_color, action, node->depth + 1, new_node_chesses);
        node->children.push_back(new_node);
        return new_node;
    }

    // 从该节点进行随机模拟，结果
    int simulate(TreeNode* node)
    {
        int new_color = node->color;
        int new_chesses = node->chesses - 1;
        if (new_chesses == 0) {
            new_color = -new_color;
            new_chesses = 2;
        }
        Game* simulate_game = new Game(node->state, new_color, new_chesses);               // 虚拟游戏初始化
        int action = node->action;                                             // 该节点的最后一步
        set<int>* legal_actions = simulate_game->cur_Board->get_legal_actions(); // 拿到该节点的合法落子集合
        set<int>::const_iterator it(legal_actions->begin());                     // 集合指针（用于集合的随机选择）
        while (!legal_actions->empty()) // 该节点已无合法落子,即直到棋盘下满
        {//每个循环只走一子
            it = legal_actions->begin();
            advance(it, rand() % legal_actions->size()); // 随机选择一步
            action = *it;
            legal_actions->erase(action);                // 删去这一步
            simulate_game->make_move(action);            // 做出这一步
            int x = action / GRID_SIZE, y = action % GRID_SIZE;
            for (int i = 0; i < 8; i++)
            {
                int tx = x + mov[i][0], ty = y + mov[i][1];
                if (tx < 0 || tx >= GRID_SIZE || ty < 0 || ty >= GRID_SIZE)
                    continue;
                if (simulate_game->cur_Board->map[tx][ty] == 2)
                    legal_actions->insert(tx * GRID_SIZE + ty);
            }
        }
        int winner = game->is_end(simulate_game->cur_Board, action); // 获取赢家（若无赢家，返回0）
        int reward = winner;                                    // 胜者为己方（1），对方（-1），和局（0）
        return reward;                                               // 返回奖励点
    }

    // 从该节点反向传播
    void back_propagate(TreeNode* node, int reward, double diff_value)
    {
        int node_depth = node->depth;
        while (node != NULL)
        {
            node->visits++;                                    // 节点的被访问次数+1
            node->reward += reward * node->color;              // 节点的奖励点更新
            if (node->parent && node->max_depth > node->parent->max_depth) 
                node->parent->max_depth = node->max_depth;     // 节点最大深度（用于调试）
            node = node->parent;                               // 向上遍历
            if(node && node->parent) 
                node->diff_value += (diff_value * node->color) / node_depth;  // 节点的获得路价值更新
        }
    }

public:
    Game* game;       // AI看到的游戏
    TreeNode* root;   // 蒙特卡洛树根
    int max_times;    // 最大迭代次数
    int player_color; // 玩家颜色
    double c;         // UCB超参数
};

int main()
{
    srand(time(0));
    Game* main_game = new Game();
    int x0, y0, x1, y1;
    int last_action = -1; // 记录最后两步（用于创建蒙特拉洛树根）
    // 分析自己收到的输入和自己过往的输出，并恢复棋盘状态
    int turnID;
    bool firstself = false;
    cin >> turnID;
    main_game->cur_player = GRID_OPPO; // 不在乎执白还是执黑，己方的颜色为1
    for (int i = 0; i < turnID; i++)
    {
        // 根据这些输入输出逐渐恢复状态到当前回合
        cin >> x0 >> y0 >> x1 >> y1;
        last_action = x1 * GRID_SIZE + y1;
        if (x0 == -1)
        {
            // 第一回合收到坐标是-1, -1，说明我是黑方（第一回合落一子的角色）
            main_game->cur_player = GRID_SELF;
            firstself = true;
            main_game->make_move(x0, y0);
            main_game->make_move(x1, y1);
        }
        if (x0 >= 0)
        {
            // 模拟对方落子
            main_game->make_move(x0, y0);
            main_game->make_move(x1, y1);
        }
        if (i < turnID - 1)
        {
            cin >> x0 >> y0 >> x1 >> y1;
            if (x0 >= 0)
            {
                // 模拟己方落子
                main_game->make_move(x0, y0);
                main_game->make_move(x1, y1);

            }
        }
    }

    // 先手特判
    if (turnID == 1 && 1 == firstself)
    {
        cout << GRID_SIZE / 2 << ' ' << GRID_SIZE / 2 << ' ' << -1 << ' ' << -1 << endl;
        return 0;
    }

    // 空余两空判断
    set<int>* main_legal_actions = main_game->cur_Board->get_legal_actions();
    if (main_legal_actions->size() == 2)
    {
        int action_1 = *main_legal_actions->begin();
        int action_2 = *main_legal_actions->end();
        cout << action_1 / GRID_SIZE << ' ' << action_1 % GRID_SIZE << ' ' << action_2 / GRID_SIZE << ' ' << action_2 % GRID_SIZE << endl;
        return 0;
    }
    /************************************************************************************/
    /***在下面填充你的代码，决策结果（本方将落子的位置）存入startX、startY、resultX、resultY中*****/
    // 下面仅为随机策略的示例代码，且效率低，可删除
    AIPlayer* AI = new AIPlayer(main_game, last_action); // 创建AI
    pair<int, int> pair_actions = AI->mcts();                             // 从蒙特卡洛树取得结果
    int best_action_1 = pair_actions.first, best_action_2 = pair_actions.second;
    int startX = best_action_1 / GRID_SIZE, startY = best_action_1 % GRID_SIZE, resultX = best_action_2 / GRID_SIZE, resultY = best_action_2 % GRID_SIZE;
    /****在上方填充你的代码，决策结果（本方将落子的位置）存入startX、startY、resultX、resultY中****/
    /************************************************************************************/

    // 决策结束，向平台输出决策结果
    cout << startX << ' ' << startY << ' ' << resultX << ' ' << resultY << endl;
    return 0;
}
