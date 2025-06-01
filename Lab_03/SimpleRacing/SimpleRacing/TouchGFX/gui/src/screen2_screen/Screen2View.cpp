#include <gui/screen2_screen/Screen2View.hpp>
#include "cmsis_os.h"
extern "C" {
#include "main.h"
#include "stm32f4xx_hal_gpio.h"
}

extern osMessageQueueId_t Queue1Handle;


Screen2View::Screen2View()
{
    tickCount = 0;
    carX = 120;      // Vị trí ban đầu giữa màn hình
    carY = 250;      // Gần đáy màn hình
    score = 0;
}

void Screen2View::setupScreen()
{
    localImageX = presenter->GetImageX();
    Screen2ViewBase::setupScreen();

    // Khởi tạo GPIO cho nút bấm PG2 và PG3
    __HAL_RCC_GPIOG_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    // Set vị trí ban đầu của xe (image1 là xe của mày)
    image1.setX(carX);
    image1.setY(carY);

    // Vị trí ban đầu của chướng ngại vật
    lamb.setX(14);
    lamb.setY(0);
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
    presenter->UpdateImageX(localImageX);
}

void Screen2View::handleTickEvent()
{
    Screen2ViewBase::handleTickEvent();

    // Xử lý điều khiển xe
    handleCarControl();

    // Animation đường đua (giữ nguyên code cũ)
    tickCount++;
    switch (tickCount % 5)
    {
    case 0:
        track0.setVisible(true);
        track4.setVisible(false);
        break;
    case 1:
        track1.setVisible(true);
        track0.setVisible(false);
        break;
    case 2:
        track2.setVisible(true);
        track1.setVisible(false);
        break;
    case 3:
        track3.setVisible(true);
        track2.setVisible(false);
        break;
    case 4:
        track4.setVisible(true);
        track3.setVisible(false);
        break;
    }

    // Di chuyển chướng ngại vật từ trên xuống
    int lambY = tickCount * 2 % 320;
    lamb.setY(lambY);

    // Thay đổi làn đường của chướng ngại vật
    int lane = (tickCount * 2 / 320) % 4;
    lamb.setX(lane * 60 + 15);

    // Kiểm tra va chạm
    checkCollision();

    // Tăng điểm
    score++;

    invalidate();
}

void Screen2View::handleCarControl()
{
    // Đọc nút trái (PG2) - Di chuyển sang trái
    if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_2) == GPIO_PIN_RESET)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET); // LED test

        if (carX > 15) {  // Giới hạn trái
            carX -= 5;    // Tốc độ di chuyển
            image1.setX(carX);
        }
    }
    else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
    }

    // Đọc nút phải (PG3) - Di chuyển sang phải
    if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_RESET)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET); // LED test

        if (carX < 195) {  // Giới hạn phải (240 - 45)
            carX += 5;     // Tốc độ di chuyển
            image1.setX(carX);
        }
    }
    else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    }
}

void Screen2View::checkCollision()
{
	int carWidth = image1.getWidth();
	int carHeight = image1.getHeight();
	int lambWidth = lamb.getWidth();
	int lambHeight = lamb.getHeight();
    // Lấy vị trí hiện tại

	int carLeft = image1.getX();
	    int carRight = carLeft + carWidth;
	    int carTop = image1.getY();
	    int carBottom = carTop + carHeight;

	    int lambLeft = lamb.getX();
	        int lambRight = lambLeft + lambWidth;
	        int lambTop = lamb.getY();
	        int lambBottom = lambTop + lambHeight;

    // Kiểm tra va chạm theo hình chữ nhật
    if (carLeft < lambRight && carRight > lambLeft &&
        carTop < lambBottom && carBottom > lambTop)
    {
        // Va chạm!
        gameOver();
    }
}

void Screen2View::gameOver()
{
    // Reset game
    carX = 120;
    image1.setX(carX);
    score = 0;

    // Reset vị trí chướng ngại vật
    lamb.setY(0);

    // Có thể thêm hiệu ứng hoặc thông báo game over
    // Ví dụ: Hiện text "Game Over"
    // gameOverText.setVisible(true);

    // Hoặc gửi message
    uint8_t msg = 'G';
    osMessageQueuePut(Queue1Handle, &msg, 0, 0);
}
