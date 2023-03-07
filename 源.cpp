#include <iostream>
#include <cstdio>
#include <vector>
#include <math.h>

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
	void reset() {
		memset(map, 0, sizeof map);
	}
public:
	int map[GRID_SIZE][GRID_SIZE];
};

class TreeNode {
public:
	TreeNode() {}

	void expend() {}

	void select() {}

	void get_value() {}

	void update() {}

	bool is_leaf() { return children.size() == 0; }

	bool is_root() { return parent == -1; }
public:
	int parent;
	vector<int> children;
	int n;  // ��ģ��Ĵ���
	int xj; // win�Ĵ���
	int nj; // �ýڵ�ģ��Ĵ���
	int C;  // ����C
};

class MCTS {
public:
	MCTS() {}

	int roolout() {}

	void playout() {}

public:
	TreeNode* root;
};

class Game {
public:
	Game(int start_player) {
		// Initialize the chess board
		cur_Board = new Board;
		cur_Board->reset();
		cur_player = start_player;
	}
	// �Ϸ�����
	bool legal_move(int move) {
		int x = move / GRID_SIZE, y = move % GRID_SIZE;
		if (x < 0 || y < 0 || x >= GRID_SIZE || y >= GRID_SIZE) return false;
		if (cur_Board->map[x][y] != 0) return false;
	}
	// �ж϶Ծ��Ƿ����
	int is_end() { // Return the winner of game
		// ��������
		for (int i = 0; i < GRID_SIZE; i++) {
			int count = 0;
			int prev_color = 0;
			for (int j = 0; j < GRID_SIZE; j++) {
				//cout << "[" << i << "," << j << "] ";
				int cur_color = cur_Board->map[i][j];
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

		// ��������
		for (int j = 0; j < GRID_SIZE; j++) {
			int count = 0;
			int prev_color = 0;
			for (int i = 0; i < GRID_SIZE; i++) {
				//cout << "[" << i << "," << j << "] ";
				int cur_color = cur_Board->map[i][j];
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

		// ��б������ ��б������
		for (int layer = 0; layer < GRID_SIZE; layer++) {
			int count = 0;
			int prev_color = 0;
			for (int k = 0; k < layer + 1; k++) {
				cout << "[" << layer - k << "," << k << "] ";
				int cur_color = cur_Board->map[layer - k][k];
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
				int cur_color = cur_Board->map[layer + k][k];
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
		 
		//��б������ ��б������
		for (int layer = 1; layer < GRID_SIZE; layer++) {
			int count = 0;
			int prev_color = 0;
			for (int k = 0; k < GRID_SIZE - layer; k++) {
				//cout << "[" << GRID_SIZE - k - 1 << "," << layer + k << "] ";
				int cur_color = cur_Board->map[GRID_SIZE - k - 1][layer + k];
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
				int cur_color = cur_Board->map[k][layer + k];
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
	// ����һ�����ж϶Ծ��Ƿ����
	int is_end(int x, int y) { // Return the winner of game (by last step)
		int cur_color = cur_Board->map[x][y];
		int count, i, j;

		//��������
		i = x; count = -1;
		while (i >= 0 && cur_Board->map[i][y] == cur_color) { i--; count++; }
		i = x;
		while (i < GRID_SIZE && cur_Board->map[i][y] == cur_color) { i++; count++; }
		if (count >= N_TO_WIN) return cur_color;

		//��������
		j = y; count = -1;
		while (j >= 0 && cur_Board->map[x][j] == cur_color) { j--; count++; }
		j = y;
		while (j <= GRID_SIZE && cur_Board->map[x][j] == cur_color) { j++; count++; }
		if (count >= N_TO_WIN) return cur_color;

		//��б����
		i = x; j = y; count = -1;
		while (i >= 0 && j < GRID_SIZE && cur_Board->map[i][j] == cur_color) { i--; j++; count++; }
		i = x; j = y;
		while (i < GRID_SIZE && j > 0 && cur_Board->map[i][j] == cur_color) { i++; j--; count++; }
		if (count >= N_TO_WIN) return cur_color;

		//��б����
		i = x; j = y; count = -1;
		while (i >= 0 && j >= 0 && cur_Board->map[i][j] == cur_color) { i--; j--; count++; }
		i = x; j = y; 
		while (i < GRID_SIZE && j < GRID_SIZE && cur_Board->map[i][j] == cur_color) { i++; j++; count++; }
		if (count >= N_TO_WIN) return cur_color;

		return GRID_BLANK;
	}
	// moveת��location
	pair<int, int> move_to_location(int move) {
		int x = move / GRID_SIZE;
		int y = move % GRID_SIZE;
		return { x, y };
	}
	// locationת��move
	int location_to_move(pair<int, int> location) {
		int x = location.first;
		int y = location.second;
		return x * GRID_SIZE + y;
	}
	// ����ִ����
	void change_turn() {
		chesses = 2;
		cur_player = -cur_player;
	}
	// ����
	void make_move(int move) {
		int x = move / GRID_SIZE, y = move % GRID_SIZE;

		cur_Board->map[x][y] = cur_player;
		chesses--;
		if (chesses == 0) change_turn();
	}

public:
	Board* cur_Board; // ����
	int cur_player; // ִ���� 1 or -1
	int chesses = 1; // ʣ��������
};

int main() {
	return 0;
}