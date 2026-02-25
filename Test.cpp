#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    // 1. สร้างหน้าต่างเกม ขนาด 800x800 พิกเซล
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers 10x10 Project");

    // 2. โหลดรูปภาพกระดาน
    sf::Texture boardTexture;
    if (!boardTexture.loadFromFile("Board.png")) {
        // ถ้ารูปโหลดไม่ได้ ให้แจ้ง error
        std::cerr << "Error: Could not load board.png" << std::endl;
        return -1;
    }

    // 3. สร้าง Sprite (ตัวที่จะเอารูปไปแปะเพื่อแสดงผล)
    sf::Sprite boardSprite;
    boardSprite.setTexture(boardTexture);

    // ปรับขนาดรูปให้พอดีกับหน้าต่าง (สมมติรูปเดิมขนาดไม่เท่า 800x800)
    // ถ้าอยากให้เต็มจอเป๊ะๆ อาจต้องคำนวณ scale:
    // float scaleX = 800.0f / boardTexture.getSize().x;
    // float scaleY = 800.0f / boardTexture.getSize().y;
    // boardSprite.setScale(scaleX, scaleY);

    // --- Game Loop (วนลูปทำงานตลอดเวลา) ---
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // เช็คว่ากดปิดโปรแกรมหรือไม่
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // --- การวาดภาพ (Draw) ---
        window.clear();         // 1. ล้างหน้าจอเก่า
        window.draw(boardSprite); // 2. วาดกระดาน
        // (เดี๋ยวเราจะมาเพิ่มโค้ดวาดหมาก O, X ตรงนี้)
        window.display();       // 3. แสดงผลขึ้นหน้าจอ
    }

    return 0;
}
