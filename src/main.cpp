#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>
#include <random>
#include <string>
#include <chrono>

using namespace std::string_literals;
sf::Font font1;

// settings which change a lot of stuff
constexpr float WIDTH {1400};
constexpr float HEIGHT {900};
constexpr float cutoff = 14;
constexpr unsigned long long CROSS_RATE = 1000000000;
constexpr unsigned long long DEATH_TIME = CROSS_RATE*12;
constexpr unsigned int MIN_BLOBS = 8;
constexpr unsigned int MAX_BLOBS = 24;
constexpr unsigned int SPEED = 32;


std::uniform_int_distribution<> xmove(45, WIDTH -45);
std::uniform_int_distribution<> ymove(45, HEIGHT-45);

std::uniform_int_distribution<> small_dis(10, 20);
std::uniform_int_distribution<> big_dist(35, 45);

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> binary_dis(0, 1);

using sf::CircleShape;
/* using sf::RectangleShape; */



enum Color{
  Green, // dominant
  Yellow,
};
enum Size{
  Big, // dominant
  Small,
};

typedef std::pair<Color, Color> ColorGenotype;
typedef std::pair<Size , Size > SizeGenotype;

sf::Color hex2rgb(const int hex){
  return sf::Color{static_cast<sf::Uint8>(((hex >> 16) & 0xFF)), static_cast<sf::Uint8>(((hex >> 8) & 0xFF)), static_cast<sf::Uint8>((hex & 0xFF))};
}

class Blob{

  public:
    Blob(sf::Vector2f=sf::Vector2f{0, 0}, ColorGenotype c=ColorGenotype{Green, Green}, SizeGenotype sz=SizeGenotype{Big, Big});
    Blob(Blob &&) = default;
    Blob(const Blob &) = default;
    Blob(Blob &parent1, Blob &parent2);
    Blob &operator=(Blob &&) = default;
    Blob &operator=(const Blob &) = default;
    ~Blob() = default;

    ColorGenotype c;
    SizeGenotype s;
    sf::Vector2f vector;
    sf::Vector2f dest;
    float size;
    CircleShape object;

    sf::Text coltext;
    sf::Text sizetext;

    std::chrono::steady_clock::time_point previous_cross;
    std::chrono::steady_clock::time_point birth;
    std::chrono::steady_clock::time_point move;

    void goCord(int x, int y);
    void update();
    bool operator==(Blob &another);

  private:

};

void Blob::goCord(int x, int y){
  /* if(y<=0)           this->vector.y=4; */
  /* else if(y>=HEIGHT) this->vector.y=HEIGHT-4; */
  /* else               this->vector.y = y; */
  this->vector.y = y;
  /* if(x<=0)           this->vector.x=4; */
  /* else if(x>=WIDTH)  this->vector.x = WIDTH-4; */
  /* else               this->vector.x = x; */
  this->vector.x = x;
  this->object.setPosition(this->vector);
}

void Blob::update(){

  auto y = std::minmax(this->vector.y, this->dest.y);
  auto x = std::minmax(this->vector.x, this->dest.x);
  float dist = ((y.second-y.first)*(y.second-y.first)+(x.second-x.first)*(x.second-x.first));
  if(dist <= (this->size+cutoff)*(this->size+cutoff)){
    this->dest.x = xmove(gen);
    this->dest.y = ymove(gen);
  }
  else{
    if(this->vector.x < this->dest.x)
      this->goCord(this->vector.x+1, this->vector.y);
    else if(this->vector.x > this->dest.x)
      this->goCord(this->vector.x-1, this->vector.y);

    /* else  */
    /*   this->dest.x = xmove(gen); */

    if(this->vector.y < this->dest.y)
      this->goCord(this->vector.x, this->vector.y+1);
    else if(this->vector.y > this->dest.y)
      this->goCord(this->vector.x, this->vector.y-1);

    /* else  */
    /*   this->dest.y = ymove(gen); */
  }
}


bool Blob::operator==(Blob &another){
  auto y = std::minmax(this->vector.y, another.vector.y);
  auto x = std::minmax(this->vector.x, another.vector.x);
  float dist = ((y.second-y.first)*(y.second-y.first)+(x.second-x.first)*(x.second-x.first));
  return dist <= (this->size+another.size+cutoff)*(this->size+another.size+cutoff);
  /* float dist = std::sqrtf((y.second-y.first)*(y.second-y.first)+(x.second-x.first)*(x.second-x.first)); */
  /* return dist <= (this->size+another->size+cutoff); */
}

Blob::Blob(sf::Vector2f v, ColorGenotype c, SizeGenotype sz){

  this->c=c;
  this->s=sz;

  if(sz.first==Big || sz.second==Big) this->size = big_dist(gen);
  else                                this->size = small_dis(gen);


  std::string buf;
  buf.reserve(4);
  if(this->c.first==Green)  buf+="G";
  else                      buf+="g";
  if(this->c.second==Green) buf+="G";
  else                      buf+="g";

  this->coltext=sf::Text(buf, font1, this->size);
  buf.clear();

  if(this->s.first==Big)  buf+="B";
  else                    buf+="b";
  if(this->s.second==Big) buf+="B";
  else                    buf+="b";

  this->sizetext=sf::Text(buf, font1, this->size);
  buf.clear();



  this->previous_cross = std::chrono::steady_clock::now();
  this->birth          = std::chrono::steady_clock::now();
  this->vector=v;
  this->dest.x = xmove(gen);
  this->dest.y = ymove(gen);
  this->object.setPosition(v);
  this->object=CircleShape{size};
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

Blob::Blob(Blob &parent1, Blob &parent2){

  bool order = binary_dis(gen)==0;

  Color c1 = binary_dis(gen)==0?parent1.c.first:parent1.c.second;
  Color c2 = binary_dis(gen)==0?parent2.c.first:parent2.c.second;
  this->c=ColorGenotype{
    order?c1:c2, order?c2:c1
  };

  Size sz1 = binary_dis(gen)==0?parent1.s.first:parent1.s.second;
  Size sz2 = binary_dis(gen)==0?parent2.s.first:parent2.s.second;
  this->s=SizeGenotype{
    order?sz1:sz2, order?sz2:sz1
  };

  if(this->s.first==Big || this->s.second==Big) this->size = big_dist(gen);
  else                                          this->size = small_dis(gen);


  std::string buf;
  buf.reserve(4);
  if(this->c.first==Green)  buf+="G";
  else                      buf+="g";
  if(this->c.second==Green) buf+="G";
  else                      buf+="g";

  this->coltext=sf::Text(buf, font1, this->size);
  buf.clear();

  if(this->s.first==Big)    buf+="B";
  else                      buf+="b";
  if(this->s.second==Big)   buf+="B";
  else                      buf+="b";

  this->sizetext=sf::Text(buf, font1, this->size);
  buf.clear();



  this->previous_cross = std::chrono::steady_clock::now();
  this->birth          = std::chrono::steady_clock::now();
  this->vector = parent1.vector;
  this->dest.x = xmove(gen);
  this->dest.y = ymove(gen);
  this->object = CircleShape{this->size};
  this->object.setPosition(parent1.vector);
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
  blobs.push_back(Blob{sf::Vector2f{(WIDTH/2), (HEIGHT/2)+(HEIGHT/4)}, ColorGenotype{Yellow, Yellow}, SizeGenotype{Small, Small}});
  blobs.push_back(Blob{sf::Vector2f{(WIDTH/2), (HEIGHT/2)-(HEIGHT/4)}, ColorGenotype{Green , Green }, SizeGenotype{Big  , Big  }});

  while (w.isOpen()) {
    sf::Event event;

    while (w.pollEvent(event)) {
      if(event.type == sf::Event::Closed)
        w.close();
      if(event.type == sf::Event::KeyPressed){
        if(event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Q) w.close();

        if(event.key.code == sf::Keyboard::Right) blobs[0].goCord(blobs[0].vector.x+SPEED, blobs[0].vector.y  );
        if(event.key.code == sf::Keyboard::Left)  blobs[0].goCord(blobs[0].vector.x-SPEED, blobs[0].vector.y  );
        if(event.key.code == sf::Keyboard::Up)    blobs[0].goCord(blobs[0].vector.x,    blobs[0].vector.y-SPEED);
        if(event.key.code == sf::Keyboard::Down)  blobs[0].goCord(blobs[0].vector.x,    blobs[0].vector.y+SPEED);

      }
    }



    w.clear(sf::Color{0x282828ff});
    for(size_t i=0;i<blobs.size();i++){
      if(blobs.size()<=MAX_BLOBS){
        for(size_t j=i;j<blobs.size();j++){
          if(i==j) continue;
          if(blobs[i]==blobs[j]){
            auto b = std::chrono::steady_clock::now()-blobs[i].previous_cross;
            auto c = std::chrono::steady_clock::now()-blobs[j].previous_cross;
            if(b.count()>=CROSS_RATE && c.count()>=CROSS_RATE){
              blobs.push_back(Blob(blobs[i], blobs[j]));
              blobs[i].previous_cross=std::chrono::steady_clock::now();
              blobs[j].previous_cross=std::chrono::steady_clock::now();
            }
          }
        }
      }


      blobs[i].update();
      blobs[i].coltext.setPosition(blobs[i].vector.x+blobs[i].size, blobs[i].vector.y);
      blobs[i].sizetext.setPosition(blobs[i].vector.x, blobs[i].vector.y+blobs[i].size);
      w.draw(blobs[i].object);
      w.draw(blobs[i].coltext);
      w.draw(blobs[i].sizetext);
    }

    if(blobs.size()>MIN_BLOBS){
      for(size_t i=0;i<blobs.size();i++){
        auto a = std::chrono::steady_clock::now()-blobs[i].birth;
        if(a.count()>=DEATH_TIME)
          blobs.erase(blobs.begin()+i);
      }
    }
    w.display();
  }
  return 0;
}
