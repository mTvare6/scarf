#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <random>
#include <string>
#include <chrono>
#include <variant>

using namespace std::string_literals;
sf::Font font1;

// settings which change a lot of stuff
constexpr float WIDTH {1400};
constexpr float HEIGHT {900};
constexpr float cutoff = 14;
#define CROSS_RATE 1000000000
std::uniform_int_distribution<> motion_dis(-6, 6);
std::uniform_int_distribution<> small_dis(5, 20);
std::uniform_int_distribution<> big_dist(30, 50);

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> binary_dis(0, 1);



enum Color{
  Green, // dominant
  Yellow,
};
enum Size{
  Big,
  Small,
};

typedef std::pair<Color, Color> ColorGenotype;
typedef std::pair<Size , Size > SizeGenotype;

sf::Color hex2rgb(const int hex){
  return sf::Color{static_cast<sf::Uint8>(((hex >> 16) & 0xFF)), static_cast<sf::Uint8>(((hex >> 8) & 0xFF)), static_cast<sf::Uint8>((hex & 0xFF))};
}

class Blob
{
  public:
    Blob(sf::Vector2f=sf::Vector2f{0, 0}, ColorGenotype c=ColorGenotype{Green, Green}, SizeGenotype sz=SizeGenotype{Big, Big});
    Blob(Blob &&) = default;
    Blob(const Blob &) = default;
    Blob(Blob *parent1, Blob *parent2);
    Blob &operator=(Blob &&) = default;
    Blob &operator=(const Blob &) = default;
    ~Blob() = default;

    ColorGenotype c;
    SizeGenotype sz;
    sf::Vector2f vector;
    float size;
    sf::CircleShape object;

    sf::Text coltext;
    sf::Text sizetext;

    std::chrono::steady_clock::time_point previous_cross;

    void update(int x, int y);
    bool isTouching(Blob *another);

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
  return dist <= (this->size+another->size+cutoff)*(this->size+another->size+cutoff);
  /* float dist = std::sqrtf((y.second-y.first)*(y.second-y.first)+(x.second-x.first)*(x.second-x.first)); */
  /* return dist <= (this->size+another->size+cutoff); */
}

Blob::Blob(sf::Vector2f v, ColorGenotype c, SizeGenotype sz){

  this->c=c;
  this->sz=sz;

  if(sz.first==Small || sz.second==Small) this->size = small_dis(gen);
  else                                    this->size = big_dist(gen);
  

  std::string buf;
  if(this->c.first==Green)  buf+="G";
  else                      buf+="g";
  if(this->c.second==Green) buf+="G";
  else                      buf+="g";

  this->coltext=sf::Text(buf, font1, this->size);
  buf.clear();

  if(this->sz.first==Big)    buf+="S";
  else                       buf+="s";
  if(this->sz.second==Small) buf+="S";
  else                       buf+="s";

  this->sizetext=sf::Text(buf, font1, this->size);
  buf.clear();



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

  bool order = binary_dis(gen)==0;

  Color c1 = binary_dis(gen)==0?parent1->c.first:parent1->c.second;
  Color c2 = binary_dis(gen)==0?parent2->c.first:parent2->c.second;
  this->c=ColorGenotype{
    order?c1:c2, order?c2:c1
  };

  Size sz1 = binary_dis(gen)==0?parent1->sz.first:parent1->sz.second;
  Size sz2 = binary_dis(gen)==0?parent2->sz.first:parent2->sz.second;
  this->sz=SizeGenotype{
    order?sz1:sz2, order?sz2:sz1
  };

  if(this->sz.first==Small || this->sz.second==Small) this->size = small_dis(gen);
  else                                                this->size = big_dist(gen);


  std::string buf;
  if(this->c.first==Green)  buf+="G";
  else                      buf+="g";
  if(this->c.second==Green) buf+="G";
  else                      buf+="g";

  this->coltext=sf::Text(buf, font1, this->size);
  buf.clear();

  if(this->sz.first==Big)    buf+="S";
  else                       buf+="s";
  if(this->sz.second==Small) buf+="S";
  else                       buf+="s";

  this->sizetext=sf::Text(buf, font1, this->size);
  buf.clear();



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




/* Blob::~Blob(){ */
/* } */

int main(int argc, char **argv) {
  sf::RenderWindow w(sf::VideoMode(WIDTH, HEIGHT), "", sf::Style::Resize);
  font1.loadFromFile("scientifica Nerd Font.ttf");

  std::vector<Blob> blobs;
  blobs.push_back(Blob{sf::Vector2f{(WIDTH/2), (HEIGHT/2)-(HEIGHT/4)}, ColorGenotype{Green , Green }, SizeGenotype{Big  , Big  }});
  blobs.push_back(Blob{sf::Vector2f{(WIDTH/2), (HEIGHT/2)+(HEIGHT/4)}, ColorGenotype{Yellow, Yellow}, SizeGenotype{Small, Small}});

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
      blobs[i].coltext.setPosition(blobs[i].vector);
      blobs[i].sizetext.setPosition(blobs[i].vector.x, blobs[i].vector.y+blobs[i].size);
      w.draw(blobs[i].object);
      w.draw(blobs[i].coltext);
      w.draw(blobs[i].sizetext);
    }
    w.display();
  }
  return 0;
}
