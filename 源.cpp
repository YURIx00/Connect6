#include <iostream>
#include <cstdio>
#include <string.h>
#include <random>
#include <cstdlib>
#include <ctime>
#include <set>
#include <algorithm>

#define judge_black 0
#define judge_white 1
#define GRID_SIZE 8
#define GRID_BLANK 0
#define GRID_BLACK 1
#define GRID_WHITE -1
#define N_TO_WIN 6
#define PLAYER_0 0
#define PLAYER_1 1

using namespace std;

class Board {
public:
	Board() {
		reset();
	}

	Board(Board* _board) {
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				_board->map[i][j] = map[i][j];
			}
		}
	}

	void reset() {
		memset(map, 0, sizeof map);
	}

	void make_move(int move, int color) {
		int x = move / GRID_SIZE, y = move % GRID_SIZE;
		map[x][y] = color;
	}

	set<int>* get_legal_actions() {
		set<int>* legal_actions = new set<int>;

		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				if (map[i][j] == 0) legal_actions->insert(i * GRID_SIZE + j);
			}
		}

		return legal_actions;
	}

	void operator = (Board* _board) {
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				_board->map[i][j] = map[i][j];
			}
		}
	}
public:
	int map[GRID_SIZE][GRID_SIZE];
};

class TreeNode {
public:
	TreeNode(TreeNode* _parent, Board* _state, int _color) {
		parent = _parent;
		children.resize(0);
		state = _state;
		color = _color;
	}

	bool is_full_expand() {
		set<int>* legal_actions = state->get_legal_actions();
		return legal_actions->size() == 0;
	}

	bool is_leaf() { return children.size() == 0; }

	bool is_root() { return parent == NULL; }
public:
	TreeNode* parent;
	vector<TreeNode*> children;
	Board* state;
	int color;
	int reward;
	int visits;
	int action_1, action_2;
};

class Game {
public:
	Game() {
		cur_Board = new Board;
		cur_Board->reset();
		chesses = 1;
		last_blank = GRID_SIZE * GRID_SIZE;
	}

	Game(Board* state, int color) {
		cur_Board = state;
		cur_player = color;
		chesses = 2;
	}

	// 合法落子
	bool legal_move(int move) {
		int x = move / GRID_SIZE, y = move % GRID_SIZE;
		if (x < 0 || y < 0 || x >= GRID_SIZE || y >= GRID_SIZE) return false;
		if (cur_Board->map[x][y] != 0) return false;
	}

	// 判断对局是否结束
	int is_end(Board* state) { // Return the winner of game
		// 横排六子
		for (int i = 0; i < GRID_SIZE; i++) {
			int count = 0;
			int prev_color = 0;
			for (int j = 0; j < GRID_SIZE; j++) {
				//cout << "[" << i << "," << j << "] ";
				int cur_color = state->map[i][j];
				if (cur_color != 0 && cur_color == prev_color) {
					count++;
					if (count >= N_TO_WIN) return cur_color;
				}
				else {
					count = 1;
					prev_color = cur_color;
				}
			}
			//cout << endl;
		}

		// 竖排六子
		for (int j = 0; j < GRID_SIZE; j++) {
			int count = 0;
			int prev_color = 0;
			for (int i = 0; i < GRID_SIZE; i++) {
				//cout << "[" << i << "," << j << "] ";
				int cur_color = state->map[i][j];
				if (cur_color != 0 && cur_color == prev_color) {
					count++;
					if (count >= N_TO_WIN) return cur_color;
				}
				else {
					count = 1;
					prev_color = cur_color;
				}
			}
			//cout << endl;
		}

		// 右斜六子上 左斜六子下 
		for (int layer = 0; layer < GRID_SIZE; layer++) {
			int count = 0;
			int prev_color = 0;
			for (int k = 0; k < layer + 1; k++) {
				cout << "[" << layer - k << "," << k << "] ";
				int cur_color = state->map[layer - k][k];
				if (cur_color != 0 && cur_color == prev_color) {
					count++;
					if (count >= N_TO_WIN) return cur_color;
				}
				else {
					count = 1;
					prev_color = cur_color;
				}
			}
			//cout << "*" << endl;
			for (int k = 0; k < GRID_SIZE - layer; k++) {
				//cout << "[" << layer + k << "," << k << "] ";
				int cur_color = state->map[layer + k][k];
				if (cur_color != 0 && cur_color == prev_color) {
					count++;
					if (count >= N_TO_WIN) return cur_color;
				}
				else {
					count = 1;
					prev_color = cur_color;
				}
			}
			//cout << endl;
		}

		//右斜六子下 左斜六子上
		for (int layer = 1; layer < GRID_SIZE; layer++) {
			int count = 0;
			int prev_color = 0;
			for (int k = 0; k < GRID_SIZE - layer; k++) {
				//cout << "[" << GRID_SIZE - k - 1 << "," << layer + k << "] ";
				int cur_color = state->map[GRID_SIZE - k - 1][layer + k];
				if (cur_color != 0 && cur_color == prev_color) {
					count++;
					if (count >= N_TO_WIN) return cur_color;
				}
				else {
					count = 1;
					prev_color = cur_color;
				}
			}
			//cout << "&" << endl;
			for (int k = 0; k < GRID_SIZE - layer; k++) { // k: [7 -> 1]
				int cur_color = state->map[k][layer + k];
				//cout << "[" << k << "," << layer + k << "] ";
				if (cur_color != 0 && cur_color == prev_color) {
					count++;
					if (count >= N_TO_WIN) return cur_color;
				}
				else {
					count = 1;
					prev_color = cur_color;
				}
			}
			//cout << endl;
		}

		return GRID_BLANK;
	}

	// 根据一步棋判断对局是否结束
	int is_end(Board* state, int move) { // Return the winner of game (by last step)
		int x = move / GRID_SIZE, y = move % GRID_SIZE;
		int cur_color = state->map[x][y];
		int count, i, j;

		//竖排六子
		i = x; count = -1;
		while (i >= 0 && state->map[i][y] == cur_color) { i--; count++; }
		i = x;
		while (i < GRID_SIZE && state->map[i][y] == cur_color) { i++; count++; }
		if (count >= N_TO_WIN) return cur_color;

		//横排六子
		j = y; count = -1;
		while (j >= 0 && state->map[x][j] == cur_color) { j--; count++; }
		j = y;
		while (j <= GRID_SIZE && state->map[x][j] == cur_color) { j++; count++; }
		if (count >= N_TO_WIN) return cur_color;

		//右斜六子
		i = x; j = y; count = -1;
		while (i >= 0 && j < GRID_SIZE && state->map[i][j] == cur_color) { i--; j++; count++; }
		i = x; j = y;
		while (i < GRID_SIZE && j > 0 && state->map[i][j] == cur_color) { i++; j--; count++; }
		if (count >= N_TO_WIN) return cur_color;

		//左斜六子
		i = x; j = y; count = -1;
		while (i >= 0 && j >= 0 && state->map[i][j] == cur_color) { i--; j--; count++; }
		i = x; j = y;
		while (i < GRID_SIZE && j < GRID_SIZE && state->map[i][j] == cur_color) { i++; j++; count++; }
		if (count >= N_TO_WIN) return cur_color;

		return GRID_BLANK;
	}

	void make_move(int move) {
		cur_Board->make_move(move, cur_player);
		chesses--;
		if (chesses == 0) change_turn();
	}

	void make_move(int x, int y) {
		int move = x * GRID_SIZE + y;
		cur_Board->make_move(move, cur_player);
		chesses--;
		if (chesses == 0) change_turn();
	}

	// 更换执棋者
	void change_turn() {
		chesses = 2;
		cur_player = -cur_player;
	}

public:
	Board* cur_Board;
	int cur_player; // 执棋者 1 or -1
	int chesses; // 剩余落子数
	int last_blank;
};


class AIPlayer {
public:
	AIPlayer(Game* _game) {
		game = _game;
		root = new TreeNode(NULL, _game->cur_Board, _game->cur_player);
		max_times = 200;
		c = 2;
	}

	pair<int, int> mcts() {
		int reward;
		TreeNode* leave_node, * best_child = root;
		for (int t = 0; t < max_times; t++) {
			leave_node = select_expand_node(root);
			reward = simulate(leave_node);
			back_propagate(leave_node, reward);
			best_child = select(root);
		}
		return { best_child->action_1, best_child->action_2 };
	}

	TreeNode* select_expand_node(TreeNode* node) {
		while (game->is_end(node->state)) {
			if (node->children.size() == 0 || !node->is_full_expand()) {
				return expand(node);
			}
			else {
				node = select(node);
			}
		}
	}

	TreeNode* select(TreeNode* node) {
		double max_ucb = 0;
		vector<TreeNode*> select_children;

		for (auto child : node->children) {
			if (child->visits == 0) {
				return child;
			}

			double ucb = (double)child->reward / (double)child->visits + c * sqrt(2.0 * log((double)node->visits) / (double)child->visits);
			if (ucb == max_ucb) {
				select_children.push_back(child);
			}
			else if (ucb > max_ucb) {
				select_children = { child };
				max_ucb = ucb;
			}
		}

		int n = (int)select_children.size();
		if (n == 0) return node->parent;

		return select_children[rand() % n];
	}

	TreeNode* expand(TreeNode* node) {
		set<int>* legal_actions = node->state->get_legal_actions();
		if (legal_actions->empty()) {
			return node->parent;
		}

		//新节点的action
		for (auto child : node->children) {
			legal_actions->erase(child->action_1);
			legal_actions->erase(child->action_2);
		}
		set<int>::const_iterator it(legal_actions->begin());
		advance(it, rand() % legal_actions->size());
		int action_1 = *it;
		legal_actions->erase(action_1);
		it = legal_actions->begin();
		advance(it, rand() % legal_actions->size());
		int action_2 = *it;

		//新节点的state
		Board* new_state = new Board(node->state);
		new_state->make_move(action_1, -node->color);
		new_state->make_move(action_2, -node->color);

		TreeNode* new_node = new TreeNode(node, new_state, -node->color);
		return new_node;
	}

	int simulate(TreeNode* node) {
		Game* simulate_game = new Game(node->state, -node->color);
		int color = -node->color;
		int action = node->action_2;
		set<int>* legal_actions = simulate_game->cur_Board->get_legal_actions();
		set<int>::const_iterator it(legal_actions->begin());
		while (game->is_end(simulate_game->cur_Board, action) || legal_actions->size() == 0) {
			if (!legal_actions->empty()) {
				it = legal_actions->begin();
				advance(it, legal_actions->size());
				action = *it;
				legal_actions->erase(action);
				simulate_game->make_move(action);
			}
		}
		int winner = game->is_end(simulate_game->cur_Board, action);
		int reward = winner * 10;
		return reward;
	}

	void back_propagate(TreeNode* node, int reward) {
		while (node != NULL) {
			node->visits++;
			node->reward += reward;
			node = node->parent;
		}
	}
public:
	Game* game;
	TreeNode* root;
	int max_times; // 最大迭代次数
	int player_color; // 玩家颜色
	double c; // UCB超参数
};

int main()
{
	Game* main_game = new Game();
	int x0, y0, x1, y1;

	// 分析自己收到的输入和自己过往的输出，并恢复棋盘状态
	int turnID;
	cin >> turnID;
	main_game->cur_player = GRID_WHITE; // 先假设自己是白方
	for (int i = 0; i < turnID; i++) {
		// 根据这些输入输出逐渐恢复状态到当前回合
		cin >> x0 >> y0 >> x1 >> y1;
		if (x0 == -1) {
			main_game->cur_player = GRID_BLACK; // 第一回合收到坐标是-1, -1，说明我是黑方
			main_game->make_move(x1, y1);
		}
		if (x0 >= 0) {
			// ProcStep(x0, y0, x1, y1, -currBotColor, false); // 模拟对方落子
			main_game->make_move(x0, y0);
			main_game->make_move(x1, y1);
		}
		if (i < turnID - 1) {
			cin >> x0 >> y0 >> x1 >> y1;
			if (x0 >= 0) {
				// ProcStep(x0, y0, x1, y1, currBotColor, false); // 模拟己方落子
				main_game->make_move(x0, y0);
				main_game->make_move(x1, y1);
			}
		}
	}

	/************************************************************************************/
	/***在下面填充你的代码，决策结果（本方将落子的位置）存入startX、startY、resultX、resultY中*****/
	//下面仅为随机策略的示例代码，且效率低，可删除
	AIPlayer* AI = new AIPlayer(main_game);
	pair<int, int> pair_actions = AI->mcts();
	int action_1 = pair_actions.first, action_2 = pair_actions.second;
	int startX = action_1 / GRID_SIZE, startY = action_1 % GRID_SIZE, resultX = action_2 / GRID_SIZE, resultY = action_2 % GRID_SIZE;
	/****在上方填充你的代码，决策结果（本方将落子的位置）存入startX、startY、resultX、resultY中****/
	/************************************************************************************/

	// 决策结束，向平台输出决策结果
	cout << startX << ' ' << startY << ' ' << resultX << ' ' << resultY << endl;
	return 0;
}
