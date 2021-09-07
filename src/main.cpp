#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <random>
#include <string>
#include <chrono>
using namespace std::string_literals;


constexpr float WIDTH {1400};
constexpr float HEIGHT {900};
constexpr float cutoff = 14;
#define CROSS_RATE 1000000000

std::random_device rd; // obtain a random number from hardware
std::mt19937 gen(rd()); // seed the generator
std::uniform_int_distribution<> motion_dis(-4, 4);
std::uniform_int_distribution<> binary_dist(0, 1);

enum Color{
  Green, // green is dominant
  Yellow,
};

typedef std::pair<Color, Color> ColorGenotype;

sf::Color hex2rgb(const int hex){
  return sf::Color{static_cast<sf::Uint8>(((hex >> 16) & 0xFF)), static_cast<sf::Uint8>(((hex >> 8) & 0xFF)), static_cast<sf::Uint8>((hex & 0xFF))};
}

class Blob
{
  public:
    Blob(sf::Vector2f=sf::Vector2f{0, 0}, ColorGenotype c={ColorGenotype{Green, Green}}, float size=25);
    Blob(Blob &&) = default;
    Blob(const Blob &) = default;
    Blob(Blob *parent1, Blob *parent2);
    Blob &operator=(Blob &&) = default;
    Blob &operator=(const Blob &) = default;
    ~Blob() = default;

    ColorGenotype c;
    sf::Vector2f vector;
    float size;
    sf::CircleShape object;
    std::chrono::steady_clock::time_point previous_cross;

    void update(int x, int y);
    bool isTouching(Blob *another);
    void debug(int vectorIndex);

  private:

};

void Blob::update(int x, int y){
  if(y<=0)           this->vector.y=4;
  else if(y>=HEIGHT) this->vector.y=HEIGHT-4;
  else               this->vector.y = y;
  if(x<=0)           this->vector.x=4;
  else if(x>=WIDTH)  this->vector.x = WIDTH-4;
  else               this->vector.x = x;
  this->object.setPosition(this->vector);
}



bool Blob::isTouching(Blob *another){
  auto y = std::minmax(this->vector.y, another->vector.y);
  auto x = std::minmax(this->vector.x, another->vector.x);
  float dist = ((y.second-y.first)*(y.second-y.first)+(x.second-x.first)*(x.second-x.first));
  return dist <= (this->size+cutoff)*(this->size+cutoff);
  /* float dist = std::sqrtf((y.second-y.first)*(y.second-y.first)+(x.second-x.first)*(x.second-x.first)); */
  /* return dist <= (this->size+cutoff); */
}

Blob::Blob(sf::Vector2f v, ColorGenotype c, float size){
  this->c=c;
  this->size = size;
  this->previous_cross = std::chrono::steady_clock::now();
  this->vector=v;
  this->object.setPosition(v);
  this->object=sf::CircleShape{size};
  if(c.first==Green || c.second==Green){
    this->object.setFillColor(sf::Color{0xb8bb26ff});
    this->object.setOutlineThickness(size/10);
    this->object.setOutlineColor(sf::Color{0x98971aff});
  }
  else{
    this->object.setFillColor(sf::Color{0xfabd2fff});
    this->object.setOutlineThickness(size/10);
    this->object.setOutlineColor(sf::Color{0xd79921ff});
  }
}

Blob::Blob(Blob *parent1, Blob *parent2){

  Color c1 = binary_dist(gen)==0?parent1->c.first:parent1->c.second;
  Color c2 = binary_dist(gen)==0?parent2->c.first:parent2->c.second;
  bool order = binary_dist(gen)==0;
  this->c=ColorGenotype{
    order?c1:c2, order?c2:c1
  };
  this->size = (parent1->size+parent2->size)/2;
  this->previous_cross = std::chrono::steady_clock::now();
  this->vector = parent1->vector;
  this->object = sf::CircleShape{this->size};
  this->object.setPosition(parent1->vector);
  if(c1==Green || c2==Green){
    this->object.setFillColor(sf::Color{0xb8bb26ff});
    this->object.setOutlineThickness(this->size/10);
    this->object.setOutlineColor(sf::Color{0x98971aff});
  }
  else{
    this->object.setFillColor(sf::Color{0xfabd2fff});
    this->object.setOutlineThickness(this->size/10);
    this->object.setOutlineColor(sf::Color{0xd79921ff});
  }
}
void Blob::debug(int vectorIndex){
  std::string genome;
  if(this->c.first==Green)  genome+="G";
  else                      genome+="g";
  if(this->c.second==Green) genome+="G";
  else                      genome+="g";

  std::cout <<  "Vector" << vectorIndex << ": X: " << this->vector.x << "  Y: " << this->vector.y << "  Genome: " << genome << std::endl;
}





/* Blob::~Blob(){ */
/* } */

int main(int argc, char **argv) {
  sf::RenderWindow w(sf::VideoMode(WIDTH, HEIGHT), "", sf::Style::Resize);

  std::vector<Blob> blobs;
  blobs.push_back(Blob{sf::Vector2f{(WIDTH/2), (HEIGHT/2)-(HEIGHT/4)}, ColorGenotype{Green, Yellow},  20});
  blobs.push_back(Blob{sf::Vector2f{(WIDTH/2), (HEIGHT/2)+(HEIGHT/4)}, ColorGenotype{Green, Yellow},  20});

  while (w.isOpen()) {
    sf::Event event;

    while (w.pollEvent(event)) {
      if(event.type == sf::Event::Closed)
        w.close();
      if(event.type == sf::Event::KeyPressed){
        if(event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Q) w.close();

        if(event.key.code == sf::Keyboard::Right) blobs[1].update(blobs[1].vector.x+16, blobs[1].vector.y  );
        if(event.key.code == sf::Keyboard::Left)  blobs[1].update(blobs[1].vector.x-16, blobs[1].vector.y  );
        if(event.key.code == sf::Keyboard::Up)    blobs[1].update(blobs[1].vector.x,   blobs[1].vector.y-16);
        if(event.key.code == sf::Keyboard::Down)  blobs[1].update(blobs[1].vector.x,   blobs[1].vector.y+16);

      }
    }



    w.clear(sf::Color{0x282828ff});
    for(size_t i=0;i<blobs.size();i++){
      for(size_t j=i;j<blobs.size();j++){
        if(i==j) continue;
        auto a = std::chrono::steady_clock::now()-blobs[i].previous_cross;
        auto b = std::chrono::steady_clock::now()-blobs[j].previous_cross;
        if(blobs[i].isTouching(&blobs[j])){
          if(a.count()>=CROSS_RATE && b.count()>=CROSS_RATE){
            blobs.push_back(Blob(&blobs[i], &blobs[j]));
            blobs[i].previous_cross=std::chrono::steady_clock::now();
            blobs[j].previous_cross=std::chrono::steady_clock::now();
          }
        }
      }
      blobs[i].update(blobs[i].vector.x+motion_dis(gen), blobs[i].vector.y+motion_dis(gen)  );
      w.draw(blobs[i].object);
    }
    w.display();
  }
  return 0;
}
