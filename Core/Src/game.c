#include "st7735.h"
#include "stdlib.h"
#include "time.h"
#include "usart.h"
#include "high_scores.h"


const int BOXES_ROWS = 3;
const int BOXES_COLS = 6;

// Tak naprawde jest to 128, ale kila pixeli z boku jest zepsutych
const int SCREEN_WIDTH = 120;
const int SCREEN_HEIGHT = 160;

const int BOX_SEPARATOR = 6;

const uint16_t BACKGROUND_COLOR = ST7735_CASET;

const int MAX_POWER_UPS = 3;

int PADDLE_SPEED = 3;
int BALL_SPEED = 1;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	uint16_t color;
	int is_destroyed;
	int just_destroyed;
} Box;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	uint16_t color;
	int prev_x;
	int prev_y;
	int prev_width;
} Paddle;

typedef struct {
	int x;
	int y;
	int radius;
	uint16_t color;
	int dx;
	int dy;
	int prev_x;
	int prev_y;
} Ball;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	uint16_t color;
	int dy;
	int prev_x;
	int prev_y;
	int type;
	int is_active;
} PowerUp;


Box** create_2d_box_array(int rows, int cols) {
    // Allocate memory for rows
    Box** array = (Box**)malloc(rows * sizeof(Box*));
    if (!array) {
        exit(0);
    }

    // Allocate memory for each row
    for (int i = 0; i < rows; i++) {
        array[i] = (Box*)malloc(cols * sizeof(Box));
        if (!array[i]) {
            exit(0);
        }
    }
    return array;
}

void free_2d_box_array(Box** array, int rows) {
    for (int i = 0; i < rows; i++) {
        free(array[i]);
    }
    free(array);
}

void create_boxes(Box** boxes)
{
	srand(time(0));

	uint16_t* colors = malloc(BOXES_ROWS * sizeof(uint16_t));
	for (int row = 0; row < BOXES_ROWS; row++)
	{
		colors[row] =  (uint16_t)(rand() % 65536);
	}

	int box_width = (SCREEN_WIDTH - 20) / BOXES_COLS - BOX_SEPARATOR;
	int box_height = box_width * 2 / 3;

	for (int row = 0; row < BOXES_ROWS; row++)
	{
		for (int col = 0; col < BOXES_COLS; col++)
		{
			Box box;
			box.color = colors[row];
			box.width = box_width;
			box.height = box_height;
			box.x = (box_width + BOX_SEPARATOR) * col + 10;
			box.y = (box_height + BOX_SEPARATOR) * row + 10;
			box.is_destroyed = 0;

			boxes[row][col] = box;
		}
	}
}

int handle_input()
{
	uint8_t pressed_button;
	if (HAL_UART_Receive(&huart2, &pressed_button, 1, 5) == HAL_OK)
	{
		return pressed_button == 'a' ? -1 : 1;
	}

	return 0;
}

int hit = 0;

int ball_colide_with_box(Ball* ball, Box box, char* hit_type) {
    // Check for overlap along the x-axis
	 if (ball->x + ball->radius <= box.x || box.x + box.width <= ball->x)
	 {
		return 0; // No collision
	 }

	// Check for overlap along the y-axis
	if (ball->y + ball->radius <= box.y || box.y + box.height <= ball->y)
	{
		return 0; // No collision
	}

	hit++;
	if (hit > 5)
	{
		int a = 0;
	}


//    if ((box.x + box.width >= ball->x - (ball->radius /2) || ball->x + (ball->radius / 2) <= box.x)
//    		&& (
//    				(box.y + 1 <= ball->y - (ball->radius /2) && box.y + box.height -1 >= ball->y - (ball->radius / 2))
//				 || (box.y + 1 <= ball->y + (ball->radius /2) && box.y + box.height -1 >= ball->y + (ball->radius / 2))
//	)) {
//        *hit_type = 'S'; // Side collision
//    } else {
//        *hit_type = 'T'; // Top or Bottom collision
//    }
    if ((box.x + box.width >= ball->x || ball->x + ball->radius >= box.x)
    		&& (
    				(box.y + 2 <= ball->y  && box.y + box.height -2 >= ball->y)
				 || (box.y + 2 <= ball->y + ball->radius && box.y + box.height -2 >= ball->y + ball->radius)
	)) {
        *hit_type = 'S'; // Side collision
    } else {
        *hit_type = 'T'; // Top or Bottom collision
    }

    return 1; // Collision occurred
}

int ball_colide_with_paddle(Ball* ball, Paddle box, char* hit_type) {
    // Check for overlap along the x-axis
	 if (ball->x + ball->radius <= box.x || box.x + box.width <= ball->x)
	 {
		return 0; // No collision
	 }

	// Check for overlap along the y-axis
	if (ball->y + ball->radius <= box.y || box.y + box.height <= ball->y)
	{
		return 0; // No collision
	}

    // Determine if the collision is on the side or top/bottom
    if (ball->y < box.y || ball->y > box.y - box.height) {
        *hit_type = 'T'; // Top or Bottom collision
    } else {
        *hit_type = 'S'; // Side collision
    }

    return 1; // Collision occurred
}

int powerup_colide_with_paddle(PowerUp* powerup, Paddle box)
{
    // Check for overlap along the x-axis
	 if (powerup->x + powerup->width <= box.x || box.x + box.width <= powerup->x)
	 {
		return 0; // No collision
	 }

	// Check for overlap along the y-axis
	if (powerup->y + powerup->height <= box.y || box.y + box.height <= powerup->y)
	{
		return 0; // No collision
	}

    return 1; // Collision occurred

}


int update(int input, Box** boxes, Paddle* paddle, Ball* ball, PowerUp* powerups, int* numberOfPowerUps)
{


	// Collision with boxes
	for (int row = 0; row < BOXES_ROWS; row++)
	{
		for (int col = 0; col < BOXES_COLS; col++)
		{
			Box* box = &boxes[row][col];
			char hit_type;
			if (!box->is_destroyed && ball_colide_with_box(ball, *box, &hit_type))
			{
				if (hit_type == 'T')
				{
					ball->dy = -ball->dy;

				}
				else if (hit_type == 'S')
				{
					ball->dx = -ball->dx;
				}

				box->just_destroyed = 1;
				box->is_destroyed = 1;

				if (*numberOfPowerUps < MAX_POWER_UPS && rand()% 2 == 0)
				{
					PowerUp powerup;
					powerup.color = ST7735_MAGENTA;
					powerup.dy = 1;
					powerup.height = 5;
					powerup.width = 5;
					powerup.x = box->x;
					powerup.y = box->y;
					powerup.type = rand() % 2;
					powerup.is_active = 1;

					powerup.prev_x = powerup.x;
					powerup.prev_y = powerup.y;

					powerups[*numberOfPowerUps] = powerup;
					(*numberOfPowerUps)++;
				}
			}
		}
	}
	// Update ball position
		ball->prev_x = ball->x;
		ball->prev_y = ball->y;
		ball->x += ball->dx * BALL_SPEED;
		ball->y += ball->dy * BALL_SPEED;

		// handle screen bounds
		if ((ball->x - ball->radius) < 0 || ball->x > SCREEN_WIDTH)
		{
			ball->dx = -ball->dx;
		}

		if (ball->y < 0)
		{
			ball->dy = -ball->dy;
		}

		// if ball is outside of screem
		if (ball->y > SCREEN_HEIGHT)
		{
			return 1;
		}


	// If there are no boxes left then it's end
	// Collision with boxes
	int boxes_left = 0;
	for (int row = 0; row < BOXES_ROWS; row++)
	{
		for (int col = 0; col < BOXES_COLS; col++)
		{
			Box* box = &boxes[row][col];
			if (!box->is_destroyed)
			{
				boxes_left++;
			}
		}
	}

	if (boxes_left == 0)
	{
		return 1;
	}


	// Paddle movement
	paddle->prev_width = paddle->width;
	paddle->prev_x = paddle ->x;
	paddle->x += PADDLE_SPEED * input ;

	if (paddle->x + paddle->width > SCREEN_WIDTH)
	{
		paddle->x = SCREEN_WIDTH - paddle->width;
	}
	else if ((paddle->x) < 0)
	{
		paddle->x = 0;
	}


	// Collision with paddle
	char hit_type;
	if (ball_colide_with_paddle(ball, *paddle, &hit_type))
	{
		if (hit_type == 'T')
		{
			ball->dy = -ball->dy;

		}
		else if (hit_type == 'S')
		{
			ball->dx = -ball->dx;
		}
	}

	// Move PowerUps
	for(int i = 0; i<*numberOfPowerUps; i++)
	{
		PowerUp* powerup = &powerups[i];
		powerup->prev_x = powerup->x;
		powerup->prev_y = powerup->y;
		powerup->y += powerup->dy;

		if (powerup->is_active == 1 && powerup_colide_with_paddle(powerup, *paddle))
		{
			powerup->is_active = 0;
			if (powerup->type == 0)
			{
				paddle->prev_width = paddle->width;
				paddle->width += 2;
			}
			else if (powerup->type == 1)
			{
				paddle->prev_width = paddle->width;
				paddle->width -= 1;
			}
		}
	}

	return 0;
}


void draw_box(Box box)
{
	if (box.just_destroyed)
	{
		ST7735_FillRectangle(box.x, box.y, box.width, box.height, BACKGROUND_COLOR);
	}

	if (!box.is_destroyed)
	{
		ST7735_FillRectangle(box.x, box.y, box.width, box.height, box.color);
	}
}

void draw_powerup(PowerUp powerup)
{
	ST7735_FillRectangle(powerup.prev_x, powerup.prev_y, powerup.width, powerup.height, BACKGROUND_COLOR);
	if (powerup.is_active)
	{
		ST7735_FillRectangle(powerup.x, powerup.y, powerup.width, powerup.height, powerup.color);
	}
}


void draw_ball(Ball* ball)
{
	ST7735_FillRectangle(ball->prev_x, ball->prev_y, ball->radius, ball->radius, BACKGROUND_COLOR);
	ST7735_FillRectangle(ball->x, ball->y, ball->radius, ball->radius, ball->color);
}

void draw_paddle(Paddle* paddle)
{
	if (paddle->prev_x != paddle->x || paddle->prev_width != paddle->width)
	{
		ST7735_FillRectangle(paddle->prev_x , paddle->prev_y, paddle->prev_width + 10, paddle->height, BACKGROUND_COLOR);
	}
	ST7735_FillRectangle(paddle->x, paddle->y, paddle->width, paddle->height, paddle->color);
}

void draw(Box** boxes, Paddle* paddle, Ball* ball, PowerUp* powerups, int numberOfPowerUps)
{
	for (int row = 0; row < BOXES_ROWS; row++)
	{
		for (int col = 0; col < BOXES_COLS; col++)
		{
			Box box = boxes[row][col];
			draw_box(box);
		}
	}


	for (int i = 0; i<numberOfPowerUps; i++)
	{
		draw_powerup(powerups[i]);
	}

	draw_paddle(paddle);
	draw_ball(ball);
}

void insert_score(uint64_t scores[], uint32_t size, uint32_t destroyed) {
    // Find the correct position for the new score
    int pos = 0;
    while (pos < size && scores[pos] >= destroyed) {
        pos++;
    }

    // If the score fits within the array, shift scores and insert it
    if (pos < size) {
        // Shift all lower scores down to make space for the new score
        for (int i = size - 1; i > pos; i--) {
            scores[i] = scores[i - 1];
        }
        // Insert the new score
        scores[pos] = destroyed;
    }
}


void save_score(Box** boxes)
{
	uint32_t destroyed = 0;
	for (int row = 0; row < BOXES_ROWS; row++)
	{
		for (int col = 0; col < BOXES_COLS; col++)
		{
			Box box = boxes[row][col];
			if (box.is_destroyed)
			{
				destroyed++;
			}
		}
	}

	uint64_t scores[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	Flash_ReadArray(FLASH_USER_START_ADDR, scores, 10);
	insert_score(scores, 10, destroyed);
	Flash_WriteArray(FLASH_USER_START_ADDR, scores, 10);
}


int game()
{
	ST7735_FillScreen(BACKGROUND_COLOR);

	Box** boxes = create_2d_box_array(BOXES_ROWS, BOXES_COLS);
	create_boxes(boxes);

	Paddle paddle;
	paddle.color = ST7735_ORANGE;
	paddle.height = 5;
	paddle.width = 40;
	paddle.prev_width = paddle.width;
	paddle.x = SCREEN_WIDTH / 2 - paddle.width / 2;
	paddle.y = SCREEN_HEIGHT * 10 / 11;
	paddle.prev_x = paddle.x;
	paddle.prev_y = paddle.y;



	Ball ball;
	ball.radius = 5;
	ball.x = paddle.x + paddle.width / 2 + ball.radius /2;
	ball.y = paddle.y - 10;
	ball.color = ST7735_RED;
	ball.dx = (rand() % 7) - 3;
	ball.dy = -(rand() % 3);
	ball.prev_x = ball.x;
	ball.prev_y = ball.y;

	PowerUp powerups[MAX_POWER_UPS];
	int numberOfPowerUps = 0;

	while (1)
	{
		int input = handle_input();
		int status = update(input, boxes, &paddle, &ball, powerups, &numberOfPowerUps);

		if (status != 0)
		{
			save_score(boxes);

			// Go to high scores
			return 2;
		}
		draw(boxes, &paddle, &ball, powerups, numberOfPowerUps);
	}
}







