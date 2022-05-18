#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;

Math::MiddleAverageFilter<float,100> fpscounter;

struct Ball
{
    sf::Vector2f p;
    sf::Vector2f dir;
    float r = 0;
    float speed = 0;
    sf::Color color = sf::Color::White;
};

void draw_ball(sf::RenderWindow& window, const Ball& ball)
{
    sf::CircleShape gball;
    gball.setRadius(ball.r);
    gball.setPosition(ball.p.x, ball.p.y);
    gball.setFillColor(ball.color);
    gball.setOrigin(ball.r, ball.r);
    window.draw(gball);
}

void move_ball(Ball& ball, float deltaTime)
{
    float dx = ball.dir.x * ball.speed * deltaTime;
    float dy = ball.dir.y * ball.speed * deltaTime;
    ball.p.x += dx;
    ball.p.y += dy;
}

void draw_fps(sf::RenderWindow& window, float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    std::string string(c);
    sf::String str(c);
    window.setTitle(str);
}

////////////////////////////////////////////////////////

//Разместил несколько функций для векторной математики, для повыешения читаемости кода
float getRandomNumber(float min, float max)
{
    static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);

    return static_cast<float>(rand() * fraction * (max - min + 1) + min);
}

float Length(sf::Vector2f& P0, sf::Vector2f& P1)
{
    return sqrt((P1.x - P0.x) * (P1.x - P0.x) + (P1.y - P0.y) * (P1.y - P0.y));
}
float Length(sf::Vector2f& P0)
{
    return sqrt(P0.x * P0.x + P0.y * P0.y);
}

sf::Vector2f Normalization(sf::Vector2f& P0, sf::Vector2f& P1)
{
    sf::Vector2f tmp;
    tmp.x = (P1.x - P0.x) / Length(P0, P1);
    tmp.y = (P1.y - P0.y) / Length(P0, P1);

    return tmp;
}
sf::Vector2f Normalization(sf::Vector2f& P0)
{
    sf::Vector2f tmp;
    tmp.x = (P0.x) / Length(P0);
    tmp.y = (P0.y) / Length(P0);

    return tmp;
}

void FrameCol(Ball& b)
{

    if (((b.p.y - b.r) <= 0) || ((b.p.y + b.r) >= WINDOW_Y))
    {
        b.dir.y = -b.dir.y;
        if ((b.p.y - b.r) <= 0)
        {
            b.p.y += b.r;
        }
        else if ((b.p.y + b.r) >= WINDOW_Y)
        {
            b.p.y -= b.r;
        }
    }

    if (((b.p.x - b.r) <= 0) || ((b.p.x + b.r) >= WINDOW_X))
    {
        b.dir.x = -b.dir.x;
        if ((b.p.x - b.r) <= 0)
        {
            b.p.x += b.r;
        }
        else if ((b.p.x + b.r) >= WINDOW_X)
        {
            b.p.x -= b.r;
        }
    }
}


void AntiCol(Ball& b1, Ball& b2, float r)
{
    auto Nr = Normalization(b1.p, b2.p);
    auto r2 = r;
    b2.p = b2.p - (Nr * r2);
    b1.p = b1.p + (Nr * r2);
}


//Обработка свойства шаров после столкновения. Используем формулу обыкновенного упругого столкновения

void ExVel(Ball& b0, Ball& b1)
{   
    //Изменяем сколяр скорости после столкновения
    float S0 = 3.14 * b0.r * b0.r;
    float S1 = 3.14 * b1.r * b1.r;
    float tmp = b0.speed;
    b0.speed = ((S0 - S1) * b0.speed + 2 * S1 * b1.speed) / (S0 + S1);
    b1.speed = ((S1 - S0) * b1.speed + 2 * S0 * tmp) / (S0 + S1);

    //Изменяем вектора скоростей после столкновения
    float V0 = Length(b0.dir);
    float V1 = Length(b1.dir);

    std::swap(b0.dir, b1.dir);

    b0.dir = Normalization(b0.dir) * V0;
    b1.dir = Normalization(b1.dir) * V1;

}
////////////////////////////////////////////////////////


int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    srand(time(NULL));

    std::vector<Ball> balls;


    // randomly initialize balls
    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS + 1) + MIN_BALLS); i++) 
    {
        Ball newBall;
        //При генерировании скорости не допускаем шары с нулевым вектором, чтобы не порождались "пожирающие" сущности
        do
        {   newBall.dir.x = (-5.0f + (rand() % 10)) / 3.0f;
            newBall.dir.y = (-5.0f + (rand() % 10)) / 3.0f;
        } while (newBall.dir.x == 0 || newBall.dir.y == 0);

        newBall.r = 5 + rand() % 5;
        //При генерации положения не допускаем появления объекта за границей окна
        newBall.p.x = getRandomNumber(newBall.r, WINDOW_X - newBall.r);
        newBall.p.y = getRandomNumber(newBall.r, WINDOW_Y - newBall.r);
        newBall.speed = 30 + rand() % 30;
        balls.push_back(newBall);
    }

    window.setFramerateLimit(60);

    sf::Clock clock;
    float lastime = clock.restart().asSeconds();

    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;

        /// <summary>
        /// TODO: PLACE COLLISION CODE HERE 
        /// объекты создаются в случайном месте на плоскости со случайным вектором скорости, имеют радиус R
        /// Объекты движутся кинетически. Пространство ограниченно границами окна
        /// Напишите обработчик столкновений шаров между собой и краями окна. Как это сделать эффективно?
        /// Массы пропорцианальны площадям кругов, описывающих объекты 
        /// Как можно было-бы улучшить текущую архитектуру кода?
        /// Данный код является макетом, вы можете его модифицировать по своему усмотрению
        
        //Поиск коллизий производим по значению вектора между двумя объектами
        auto iterFirst = balls.begin();
        auto iter = balls.begin();
        
        while (iterFirst != balls.end())
        {            
            auto& a = *iterFirst;
            //Для повышения читаемости кода будем вводить дополнительные переменные со смешенными точками координат сравниваемых объектов
            float P1x1 = a.p.x + a.r;
            float P1y1 = a.p.y + a.r;
            a.color = sf::Color::White;
            
            auto iter = iterFirst+1;
     
            while (iter != balls.end())
            {
                auto& b = *iter;

                float P2x1 = b.p.x + b.r;
                float P2y1 = b.p.y + b.r;

                
                float dl = sqrt((P1x1 - P2x1) * (P1x1 - P2x1) + (P1y1 - P2y1) * (P1y1 - P2y1)) - a.r - b.r;
                //При персечении границ шаров, запускаем обработку коллизии
                if (dl <= 0)
                {   //Дополнительный обработчик. Выталкивает объекты друг из друга
                    AntiCol(a, b, dl);
                    //Обработка упругого соударения
                    ExVel(a, b);
                    a.color = sf::Color::Red;
                    
                   
                }
                ++iter;
            } 
            //Обработка столкновения с рамкой окна или выхода за ее пределы
            FrameCol(a);

            ++iterFirst;
        }
        //////////////////////////////////////////////////////////////////////////////////////////

        for (auto& ball : balls)
        {
            move_ball(ball, deltaTime);
        }

        window.clear();
        for (const auto ball : balls)
        {
            draw_ball(window, ball);
        }

		draw_fps(window, fpscounter.getAverage());
		window.display();
    }
    return 0;
}
