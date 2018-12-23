#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>

using namespace std;

// Tetromino array
wstring tetromino[7];

// Field Dimensions
int f_width = 12;
int f_height = 18;

// Screen Dimensions
int s_width = 680;
int s_height = 720;

// Field array and shape array
unsigned char *field = nullptr;
sf::RectangleShape* cells = nullptr;

// Values for special cells
const int BORDER = 9;
const int LINE = 8;
const int EMPTY = 0;

// Colour map for tetrominos
sf::Color c[10] = {sf::Color::Black, sf::Color::Cyan, sf::Color::Blue, sf::Color(255, 165, 0),
sf::Color::Yellow, sf::Color::Green, sf::Color::Magenta, sf::Color::Red, sf::Color::Transparent, sf::Color::White };

int Rotate(int x, int y, int r)
{
	switch (r % 4)
	{
	case 0: return y * 4 + x; // 0
	case 1: return 12 + y - (x * 4); // 90
	case 2: return 15 - (y * 4) - x; // 180
	case 3: return 3 - y + (x * 4); // 270
	}

	return 0;
}

bool IsValidMove(int n_tetromino, int rotation, int x, int y)
{
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			// Index into piece
			int pi = Rotate(px, py, rotation);

			// Index into field
			int fi = (y + py) * f_width + (x + px);
			
			// Test if position already occupied
			if (x + px >= 0 && x + px < f_width &&
				y + px >= 0 && y + py < f_height &&
				tetromino[n_tetromino][pi] == L'X' && field[fi] != 0)
				return false;
		}

	return true;
}

int main()
{
	// Create tetrominos
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	// Create field array
	field = new unsigned char[f_width * f_height];
	cells = new sf::RectangleShape[f_width * f_height];
	
	// Create line vector
	vector<int> lines;

	// Create piece holder
	int hold = -1;
	bool hold_latch = false;

	// Setup window
	sf::RenderWindow window(sf::VideoMode(s_width, s_height), "TetrisClone");

	// Text variables
	sf::Font text_font;
	text_font.loadFromFile("./cour.ttf");
	sf::Text text_score;
	sf::Text text_speed;
	text_score.setFont(text_font);
	text_speed.setFont(text_font);
	text_score.setPosition(sf::Vector2f(500, 0));
	text_speed.setPosition(sf::Vector2f(500, 30));
	text_score.setCharacterSize(25);
	text_speed.setCharacterSize(25);
	text_score.setFillColor(sf::Color::White);
	text_speed.setFillColor(sf::Color::White);

	// Random Number Generation
	random_device rd; // Initialise seed engine
	mt19937 rng(rd()); // Mersenne-Twister number generator
	uniform_int_distribution<int> uni(0, 6);

	// Current piece properties
	int c_piece = 0;
	int c_rotation = 0;
	int c_x = f_width / 2;
	int c_y = 0;
	bool rotate_hold = false;

	// Game status flag
	bool game_over = false;

	// Piece movement
	int ticks = 0;
	int speed = 20;
	bool force_down = false;

	// Scoring
	int piece_count = 0;
	int score = 0;

	// Event Loop
	sf::Event event;

	// Set all cells to EMPTY unless BORDER and setup rectangles
	for (int x = 0; x < f_width; x++)
		for (int y = 0; y < f_height; y++)
		{
			field[y * f_width + x] = (x == 0 || x == f_width - 1 || y == f_height - 1) ? BORDER : EMPTY;
			cells[y * f_width + x].setSize(sf::Vector2f(40, 40));
			cells[y * f_width + x].setPosition(x * 40, y * 40);
		}
	
	// Print Controls
	std::cout << "----- CONTROLS -----" << std::endl;
	std::cout << "Left/Right/Down: Move Tetromino" << std::endl;
	std::cout << "Up: Hold/Retrieve Tetromino" << std::endl;
	std::cout << "Z: Rotate Tetromino" << std::endl;

	while (window.isOpen())
	{
		// Refresh rate  = 50ms/tick
		sf::sleep(sf::milliseconds(50));

		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed) window.close();

		if (game_over ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();

		ticks++;
		force_down = (ticks == speed);

		// GAME INPUT / LOGIC

		c_x -= (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && IsValidMove(c_piece, c_rotation, c_x - 1, c_y)) ? 1 : 0;
		c_x += (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && IsValidMove(c_piece, c_rotation, c_x + 1, c_y)) ? 1 : 0;
		c_y += (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && IsValidMove(c_piece, c_rotation, c_x, c_y + 1)) ? 1 : 0;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
		{
			c_rotation += (!rotate_hold && IsValidMove(c_piece, c_rotation + 1, c_x, c_y)) ? 1 : 0;
			rotate_hold = true;
		}
		else
			rotate_hold = false;

		// Hold piece
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{			
			if (!hold_latch)
			{
				// Hold current piece
				int temp_hold = c_piece;

				// Generate new piece
				c_piece = (hold == -1) ? uni(rng) : hold;
				c_rotation = 0;
				c_x = f_width / 2;
				c_y = 0;

				hold = temp_hold;
			}

			hold_latch = true;
		}
		else
			hold_latch = false;

		if (force_down)
		{
			ticks = 0;

			if (IsValidMove(c_piece, c_rotation, c_x, c_y + 1))
				c_y += 1;
			else
			{
				// Increase difficulty every 10 pieces
				piece_count++;
				if (piece_count % 10 == 0)
					if (speed >= 1) speed--;

				// Lock current piece into the field
				for (int x = 0; x < 4; x++)
					for (int y = 0; y < 4; y++)
						if (tetromino[c_piece][Rotate(x, y, c_rotation)] == L'X')
							field[(c_y + y) * f_width + (c_x + x)] = c_piece + 1;

				// Check for full lines
				for (int y = 0; y < 4; y++)
					if (c_y + y < f_height - 1)
					{
						bool line = true;
						for (int x = 1; x < f_width - 1; x++)
							line &= (field[(c_y + y) * f_width + x]) != 0;

						if (line)
						{
							for (int x = 1; x < f_width - 1; x++)
								field[(c_y + y) * f_width + x] = LINE;
							lines.push_back(c_y + y);
						}
					}

				// Scoring
				score += 25;
				if (!lines.empty())
					score += (1 << lines.size()) * 100;

				// Choose next piece
				c_piece = uni(rng);
				c_rotation = 0;
				c_x = f_width / 2;
				c_y = 0;

				// If piece doesn't fit, game over
				game_over = !IsValidMove(c_piece, c_rotation, c_x, c_y + 1);
			}
		}

		// RENDER OUTPUT

		window.clear();
		
		// Update and draw field
		for (int x = 0; x < f_width; x++)
			for (int y = 0; y < f_height; y++)
			{
				cells[y * f_width + x].setFillColor(c[field[y * f_width + x]]);
				window.draw(cells[y * f_width + x]);
			}

		// Draw Current Piece
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
				if (tetromino[c_piece][Rotate(x, y, c_rotation)] == L'X')
				{
					sf::RectangleShape cell;
					cell.setSize(sf::Vector2f(40, 40));
					cell.setPosition(sf::Vector2f((c_x + x) * 40, (c_y + y) * 40));
					cell.setFillColor(c[c_piece + 1]);
					window.draw(cell);
				}

		// Draw Hold Piece
		if (hold != -1)
			for (int x = 0; x < 4; x++)
				for (int y = 0; y < 4; y++)
					if (tetromino[hold][Rotate(x, y, 0)] == L'X')
					{
						sf::RectangleShape cell;
						cell.setSize(sf::Vector2f(40, 40));
						cell.setPosition(sf::Vector2f((12 + x) * 40, (2 + y) * 40));
						cell.setFillColor(c[hold + 1]);
						window.draw(cell);
					}

		if (!lines.empty())
		{
			sf::sleep(sf::milliseconds(400));

			for (auto &l : lines)
				for (int x = 1; x < f_width - 1; x++)
				{
					for (int y = l; y > 0; y--)
						field[y * f_width + x] = field[(y - 1) * f_width + x];

					field[x] = 0;
				}

			lines.clear();
		}

		// Draw score and speed
		text_score.setString("SCORE: " + to_string(score));
		text_speed.setString("SPEED: " + to_string(21 - speed));
		window.draw(text_score);
		window.draw(text_speed);

		window.display();
	}
	return 0;
}