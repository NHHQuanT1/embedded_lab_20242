#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

protected:
    int16_t localImageX;
    uint32_t tickCount;

    // Thêm các biến và hàm mới
    int16_t carX;           // Vị trí X của xe
    int16_t carY;           // Vị trí Y của xe
    int score;              // Điểm số

    void handleCarControl();    // Xử lý điều khiển xe
    void checkCollision();      // Kiểm tra va chạm
    void gameOver();           // Xử lý game over
};

#endif // SCREEN2VIEW_HPP
