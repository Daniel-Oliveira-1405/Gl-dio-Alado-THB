#include<SFML/Graphics.hpp>
#include<windows.h>
#include<commdlg.h>
#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>

#include "file.h"

#define TC 35
#define WQ 25
#define HQ 18

#define MARCAR_PASSO    100
#define ALTO            101
#define EM_FRENTE       102
#define DIREITA         103
#define ESQUERDA        104
#define MEIA_VOLTA      105

using namespace std;
using namespace sf;

class Militar
{
public:
    Militar() : x(0), y(0), ang(0), selected(false) {}

    Militar(int x, int y, int id, int ang=0)
    : x(x), y(y), ang(ang), selected(0), id(id) {}

    int dir()
    {
        ang += 90;
        if(ang >= 360) ang-=360;
        return ang;
    }
    int esq()
    {
        ang -= 90;
        if(ang < 0) ang+=360;
        return ang;
    }
    int mv()
    {
        ang += 180;
        if(ang >= 360) ang-=360;
        return ang;
    }
    Vector2i em_frente()
    {
        if(ang == 0) y -= TC;
        if(ang == 90) x += TC;
        if(ang == 180) y += TC;
        if(ang == 270) x -= TC;

        return Vector2i(x, y);
    }
    operator=(Militar m)
    {
        x = m.x; y = m.y; ang = m.ang;
    }
    operator==(Militar m)
    {
        return x == m.x && y == m.y && ang == m.ang;
    }

    int x, y, ang, id;
    bool selected;
};

vector<Militar> carregar_tropa(string filename);
vector<vector<int>> carregar_passos(string filename);

void salvar_tropa(string filename, vector<Militar> tropa);
void salvar_passos(string filename, vector<vector<int>> passos);

int main()
{
    RenderWindow window(VideoMode(TC*WQ+200, TC*HQ), "Gládio Alado THB");

    /// FONTE --------------------------------------------------------
    Font fonte;
    fonte.loadFromFile("resources/consola.ttf");


    /// TEXTURAS -----------------------------------------------------
    Texture t_dir, t_esq, t_dois, t_seta;
    t_dir.loadFromFile("resources/seta_pe_direito.png");
    t_esq.loadFromFile("resources/seta_pe_esquerdo.png");
    t_dois.loadFromFile("resources/seta_dois_pes.png");
    t_seta.loadFromFile("resources/seta.png");

    Texture t_sv_tropa, t_sv_passos, t_ab_tropa, t_ab_passos;
    t_ab_passos.loadFromFile("resources/icon_abrir_sequencia_passos.jpg");
    t_ab_tropa.loadFromFile("resources/icon_abrir_tropa.jpg");
    t_sv_passos.loadFromFile("resources/icon_salvar_sequencia_passos.jpg");
    t_sv_tropa.loadFromFile("resources/icon_salvar_tropa.jpg");


    /// TEXTOS: EFETIVO, PASSO ATUAL, TOTAL DE PASSOS
    Text texto_efetivo, texto_passo_atual, texto_total_passos;
    texto_efetivo.setFont(fonte);
    texto_efetivo.setCharacterSize(24);
    texto_efetivo.setOutlineThickness(1.f);
    texto_efetivo.setOutlineColor(Color::Black);
    texto_efetivo.setFillColor(Color::Yellow);
    texto_efetivo.setStyle(Text::Bold);
    texto_efetivo.setPosition(36, 36);

    texto_passo_atual.setFont(fonte);
    texto_passo_atual.setCharacterSize(24);
    texto_passo_atual.setOutlineThickness(1.f);
    texto_passo_atual.setOutlineColor(Color::Black);
    texto_passo_atual.setFillColor(Color::Yellow);
    texto_passo_atual.setStyle(Text::Bold);

    texto_total_passos.setFont(fonte);
    texto_total_passos.setCharacterSize(24);
    texto_total_passos.setOutlineThickness(1.f);
    texto_total_passos.setOutlineColor(Color::Black);
    texto_total_passos.setFillColor(Color::Yellow);
    texto_total_passos.setStyle(Text::Bold);

    char buff[30];


    /// BOTÕES DE SALVAR E ABRIR
    RectangleShape btn_sv_tropa, btn_sv_passos, btn_ab_tropa, btn_ab_passos;
    btn_ab_passos.setSize(Vector2f(150, 75));
    btn_ab_tropa.setSize(Vector2f(150, 75));
    btn_sv_passos.setSize(Vector2f(150, 75));
    btn_sv_tropa.setSize(Vector2f(150, 75));

    btn_ab_passos.setTexture(&t_ab_passos);
    btn_ab_tropa.setTexture(&t_ab_tropa);
    btn_sv_passos.setTexture(&t_sv_passos);
    btn_sv_tropa.setTexture(&t_sv_tropa);

    btn_sv_tropa.setPosition(TC*WQ+25, 25);
    btn_sv_passos.setPosition(TC*WQ+25, 125);
    btn_ab_tropa.setPosition(TC*WQ+25, 225);
    btn_ab_passos.setPosition(TC*WQ+25, 325);


    /// GRADE --------------------------------------------------------
    sf::VertexArray grade(sf::Lines, 90);
    for(int i=1; i<=WQ; i++)
    {
        grade.append(Vertex(Vector2f(i*TC, 0    ), Color::Cyan));
        grade.append(Vertex(Vector2f(i*TC, TC*HQ), Color::Cyan));
    }
    for(int i=1; i<=HQ; i++)
    {
        grade.append(Vertex(Vector2f(0   , i*TC), Color::Cyan));
        grade.append(Vertex(Vector2f(TC*WQ, i*TC), Color::Cyan));
    }


    /// QUADRADO DE SELEÇÃO ------------------------------------------
    RectangleShape r_select;
    r_select.setFillColor(Color(0, 128, 255, 128));
    r_select.setOutlineColor(Color(0, 128, 255));
    r_select.setOutlineThickness(1.f);


    /// MILITARES ----------------------------------------------------
    vector<Militar> militares;

    CircleShape c_mil(16);
    c_mil.setOrigin(16,16);
    c_mil.setTexture(&t_seta);
    c_mil.setFillColor(Color::White);

    Text texto_num;
    texto_num.setCharacterSize(18.f);
    texto_num.setFont(fonte);
    texto_num.setFillColor(Color::Black);
    texto_num.setStyle(Text::Bold);

    vector<Militar> inicial;


    /// SEQUÊNCIA DE PASSOS DOS MILITARES ----------------------------
    vector<vector<int>> passos;
    int passo_atual=0;

    int qt_selected=0;


    /// FUNÇÃO PARA CALCULAR A POSIÇÃO DOS MILITARES DE ACORDO COM O PASSO ATUAL
    auto update = [&passos, &inicial, &militares, &passo_atual]()
    {
        militares = inicial;
        for(int i=0; i<militares.size(); i++)
        {
            for(int j=0; j<passo_atual; j++)
            {
                if(passos[i][j] == EM_FRENTE)
                    militares[i].em_frente();
                if(passos[i][j] == DIREITA)
                    militares[i].dir();
                if(passos[i][j] == ESQUERDA)
                    militares[i].esq();
                if(passos[i][j] == MEIA_VOLTA)
                    militares[i].mv();
            }
        }
    };


    /// ANIMAÇÃO --------------------------------------------------
    bool animate=false;
    Clock clock;


    bool running=true;
    bool number_view=true;
    Vector2i pos_up, pos_down;

    while(running)
    {
        Event e;
        while(window.pollEvent(e))
        {
            bool ctrl = Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl);
            bool shift = Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift);
            bool alt = Keyboard::isKeyPressed(Keyboard::LAlt) || Keyboard::isKeyPressed(Keyboard::RAlt);
            /// CLICOU NO 'X' (FECHAR A JANELA)
            if(e.type == Event::Closed)
                running=false;

            /// EVENTOS DO MOUSE --------------------------------------------------------------
            if(e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
            {
                pos_down = Mouse::getPosition(window);
            }
            if(e.type == Event::MouseButtonReleased && e.mouseButton.button == Mouse::Left)
            {
                pos_up = Mouse::getPosition(window);

                if(pos_up == pos_down)
                {
                    /// ABRIR ARQUIVO DE SEQUÊNCIA DE PASSOS
                    if(btn_ab_passos.getGlobalBounds().contains(pos_up.x, pos_up.y))
                    {
                        passos.clear();
                        passos = carregar_passos(open_filename());
                    }
                    /// ABRIR ARQUIVO DE CONFIGURAÇÃO DE TROPA
                    if(btn_ab_tropa.getGlobalBounds().contains(pos_up.x, pos_up.y))
                    {
                        inicial.clear();
                        militares.clear();
                        inicial = carregar_tropa(open_filename());
                        militares = inicial;

                        passos.clear();
                        for(int i=0; i<inicial.size(); i++) passos.push_back(vector<int>(0));
                    }
                    /// SALVAR SEQUÊNCIA DE PASSOS
                    if(btn_sv_passos.getGlobalBounds().contains(pos_up.x, pos_up.y))
                    {
                        salvar_passos(save_filename(), passos);
                    }
                    /// SALVAR CONFIGURAÇÃO DE TROPA
                    if(btn_sv_tropa.getGlobalBounds().contains(pos_up.x, pos_up.y))
                    {
                        salvar_tropa(save_filename(), militares);
                    }

                    if(pos_up.x > TC*WQ+16 || pos_up.y > TC*HQ+16 || pos_up.x < 0 || pos_up.y < 0)
                        continue;

                    int xx, yy;
                    xx = ((pos_up.x % TC > TC/2)? pos_up.x/TC+1 : pos_up.x/TC)*TC;
                    yy = ((pos_up.y % TC > TC/2)? pos_up.y/TC+1 : pos_up.y/TC)*TC;

                    /// SE TEM MILITAR NO LUGAR, ELE TIRA
                    bool tem=false;
                    for(int i=0; i<militares.size(); i++)
                    {
                        if(militares[i].x == xx && militares[i].y == yy)
                        {
                            if(ctrl)
                                militares[i].selected = !militares[i].selected;
                            else
                            {
                                militares.erase(militares.begin()+i);
                                inicial.erase(inicial.begin()+i);
                                passos.erase(passos.begin()+i);

                                if(passos.size() == 0) passo_atual=0;
                            }
                            tem=true;
                            break;
                        }
                    }
                    /// SE NÃO TEM MILITAR NO LUGAR, ELE COLOCA
                    if(!tem)
                    {
                        militares.push_back(Militar(xx, yy, (militares.size() > 0)? militares[militares.size()-1].id+1 : 1));
                        inicial.push_back(Militar(xx, yy, (inicial.size() > 0)? inicial[inicial.size()-1].id+1 : 1));
                        vector<int> temp;
                        for(int i=0; i<passos.size(); i++)
                            temp.push_back(MARCAR_PASSO);
                        passos.push_back(temp);
                    }
                }

                /// CASO ESTEJA SELECIONANDO MILITARES -----------------------------------------
                else
                {
                    if(!ctrl) qt_selected=0;
                    r_select.setPosition(pos_down.x, pos_down.y);
                    r_select.setSize(Vector2f(pos_up.x-pos_down.x, pos_up.y-pos_down.y));

                    for(int i=0; i<militares.size(); i++)
                    {
                        if(r_select.getGlobalBounds().contains(militares[i].x, militares[i].y))
                        {
                            militares[i].selected=true;
                            qt_selected++;
                        }
                        else if(!ctrl) militares[i].selected=false;
                    }
                }
            }

            /// EVENTOS DO TECLADO ----------------------------------------------------------------------
            if(e.type == Event::KeyPressed)
            {
                if(e.key.code == Keyboard::V)
                    number_view = !number_view;

                /// TECLA 'DELETE'
                if(e.key.code == Keyboard::Delete)
                {
                    for(int i=0; i<militares.size(); i++)
                        if(militares[i].selected)
                        {
                            militares.erase(militares.begin()+i);
                            inicial.erase(inicial.begin()+i);
                            i--;
                        }
                }

                /// 'Z' - ZERAR TODAS AS SEQUÊNCIAS DE PASSOS
                if(e.key.code == Keyboard::Z)
                {
                    /// (ctrl)? TODOS OS MILITARES : SELECIONADOS;
                    /// (shift)? PASSOS antes DO ATUAL
                    /// (alt)? PASSOS depois DO ATUAL
                    /// (!shift && !alt)? TODOS OS PASSOS

                    /// TODOS OS MILITARES OU NÃO
                    for(int i=0; i<militares.size(); i++)
                    {
                        if(militares[i].selected || ctrl)
                        {
                            /// TODOS OS PASSOS antes VIRAM MARCAR PASSO
                            if(shift)
                            {
                                for(int j=0; j<=passo_atual; j++)
                                    passos[i][j] = MARCAR_PASSO;
                            }
                            /// TODOS OS PASSOS depois VIRAM MARCAR PASSO
                            else if(alt)
                            {
                                for(int j=passo_atual; j<passos[i].size(); j++)
                                    passos[i][j] = MARCAR_PASSO;
                            }
                            /// TODOS OS PASSOS (antes e depois) viram MARCAR PASSO
                            else
                            {
                                if(ctrl)
                                {
                                    for(int j=0; j<passos[i].size(); j++)
                                        passos[i].clear();
                                    passo_atual=0;

                                    inicial = militares;
                                }
                                else
                                {
                                    for(int j=0; j<passos[i].size(); j++)
                                        passos[i][j] = MARCAR_PASSO;
                                }
                            }
                        }
                    }
                } /// Keyboard::Z

                /// '->' SETA PARA DIREITA (DIREITA VOLVER)
                if(e.key.code == Keyboard::Right)
                {
                    if(ctrl)
                    {
                        for(int i=0; i<militares.size(); i++)
                            if(militares[i].selected) militares[i].dir();
                    }
                    else
                    {
                        for(int i=0; i<militares.size(); i++)
                        {
                            if(militares[i].selected)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(DIREITA);
                                else passos[i][passo_atual] = DIREITA;
                            }
                            else if(qt_selected > 0)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(MARCAR_PASSO);
                            }
                        }

                        passo_atual++;
                    }
                }

                /// '<-' SETA PARA ESQUERDA (ESQUERDA VOLVER)
                if(e.key.code == Keyboard::Left)
                {
                    if(ctrl)
                    {
                        for(int i=0; i<militares.size(); i++)
                            if(militares[i].selected) militares[i].esq();
                    }
                    else
                    {
                        for(int i=0; i<militares.size(); i++)
                        {
                            if(militares[i].selected)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(ESQUERDA);
                                else passos[i][passo_atual] = ESQUERDA;
                            }
                            else if(qt_selected > 0)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(MARCAR_PASSO);
                            }
                        }
                        passo_atual++;
                    }
                }

                /// ^
                /// | SETA PARA CIMA (EM FRENTE)
                if(e.key.code == Keyboard::Up)
                {
                    if(ctrl)
                    {
                        for(int i=0; i<militares.size(); i++)
                            if(militares[i].selected) militares[i].em_frente();
                    }
                    else
                    {
                        for(int i=0; i<militares.size(); i++)
                        {
                            if(militares[i].selected)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(EM_FRENTE);
                                else passos[i][passo_atual] = EM_FRENTE;
                            }
                            else if(qt_selected > 0)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(MARCAR_PASSO);
                            }
                        }
                        passo_atual++;
                    }
                }

                /// | SETA PARA BAIXO (MEIA VOLTA VOLVER)
                /// v
                if(e.key.code == Keyboard::Down)
                {
                    if(ctrl)
                    {
                        for(int i=0; i<militares.size(); i++)
                            if(militares[i].selected) militares[i].mv();
                    }
                    else
                    {
                        for(int i=0; i<militares.size(); i++)
                        {
                            if(militares[i].selected)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(MEIA_VOLTA);
                                else passos[i][passo_atual] = MEIA_VOLTA;
                            }
                            else if(qt_selected > 0)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(MARCAR_PASSO);
                            }
                        }
                        passo_atual++;
                    }
                }

                /// ' ' ESPAÇO (MARCAR PASSO)
                if(e.key.code == Keyboard::Space)
                {
                    for(int i=0; i<militares.size(); i++)
                    {
                        if(passo_atual == passos[i].size())
                            passos[i].push_back(MARCAR_PASSO);
                        else if(militares[i].selected) passos[i][passo_atual] = MARCAR_PASSO;
                    }
                    passo_atual++;
                }

                /// 'A' - ALTO
                if(e.key.code == Keyboard::A && (passo_atual % 2))
                {
                    if(ctrl)
                    {
                        for(int i=0; i<militares.size(); i++)
                            if(militares[i].selected && passo_atual>0) passos[i][passo_atual-1] = ALTO;
                    }
                    else
                    {
                        for(int i=0; i<militares.size(); i++)
                        {
                            if(militares[i].selected)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(ALTO);
                                else passos[i][passo_atual] = ALTO;
                            }
                            else if(qt_selected > 0)
                            {
                                if(passo_atual == passos[i].size())
                                    passos[i].push_back(MARCAR_PASSO);
                            }
                        }
                        passo_atual++;
                    }
                    /*
                    if(passos.size() == 0 || qt_selected == 0) continue;

                    while(passos[0].size()-passo_atual < 3)
                        for(auto& v : passos) v.push_back(MARCAR_PASSO);

                    for(int i=0; i<passos.size(); i++)
                    {
                        if(militares[i].selected)
                        {
                            passos[i][passo_atual] = MARCAR_PASSO;
                            passos[i][passo_atual+1] = MARCAR_PASSO;
                            passos[i][passo_atual+2] = MARCAR_PASSO;
                            passos[i][passo_atual+3] = ALTO;
                        }
                    }
                    passo_atual+=3;
                    */
                }

                /// 'O' - AVANÇAR NOS PASSOS
                if(e.key.code == Keyboard::O)
                {
                    if(passos.size() > 0)
                    {
                        if(passo_atual < passos[0].size())
                            passo_atual++;
                    }
                }

                /// 'L' - REGREDIR NOS PASSOS
                if(e.key.code == Keyboard::L)
                {
                    if(passo_atual > 0)
                        passo_atual--;
                }

                /// '0' NÚMERO 0 - PASSO ATUAL NO INÍCIO
                if(e.key.code == Keyboard::Num0)
                {
                    passo_atual=0;
                }

                /// 'G' - ANIMAÇÃO
                if(e.key.code == Keyboard::G && passo_atual < passos[0].size())
                {
                    animate=true;
                    clock.restart();
                }

                /// 'S' - STOP ANIMATION
                if(e.key.code == Keyboard::S)
                {
                    animate=false;
                }

                /// 'T' - DESSELECIONAR TODOS OS MILITARES

            } /// Key-Pressed
        }
        ///---------------------------------------------------
        update();

        Vector2i mouse = Mouse::getPosition(window);


        window.clear();

        /// DESENHAR GRADE
        window.draw(grade);

        /// DESENHAR BOTÕES

        if(btn_ab_passos.getGlobalBounds().contains(mouse.x, mouse.y))
            btn_ab_passos.setFillColor(Color(200, 200, 200));
        else btn_ab_passos.setFillColor(Color::White);

        if(btn_ab_tropa.getGlobalBounds().contains(mouse.x, mouse.y))
            btn_ab_tropa.setFillColor(Color(200, 200, 200));
        else btn_ab_tropa.setFillColor(Color::White);

        if(btn_sv_passos.getGlobalBounds().contains(mouse.x, mouse.y))
            btn_sv_passos.setFillColor(Color(200, 200, 200));
        else btn_sv_passos.setFillColor(Color::White);

        if(btn_sv_tropa.getGlobalBounds().contains(mouse.x, mouse.y))
            btn_sv_tropa.setFillColor(Color(200, 200, 200));
        else btn_sv_tropa.setFillColor(Color::White);

        window.draw(btn_sv_tropa);
        window.draw(btn_sv_passos);
        window.draw(btn_ab_tropa);
        window.draw(btn_ab_passos);

        /// DESENHAR MILITARES
        for(int i=0; i<militares.size(); i++)
        {
            auto& m = militares[i];
            c_mil.setPosition(Vector2f(m.x, m.y));
            c_mil.setFillColor((m.selected)? Color::Cyan : Color::White);
            c_mil.setRotation(m.ang-90.f);
            c_mil.setScale(Vector2f(1.f, 1.f));

            if(animate)
            {
                float time = clock.getElapsedTime().asSeconds();
                float factor = 1.f + time/5.f;
                c_mil.setScale(Vector2f(factor, factor));

                if(passos[i][passo_atual-1] == ESQUERDA)
                    c_mil.setRotation((float)m.ang-time*2*90.f);
                if(passos[i][passo_atual-1] == DIREITA)
                    c_mil.setRotation((float)m.ang-180.f+time*2*90.f);
                if(passos[i][passo_atual-1] == MEIA_VOLTA)
                    c_mil.setRotation((float)m.ang+90.f-time*2*180.f);

                if(passos[i][passo_atual] == EM_FRENTE)
                {
                    if(m.ang == 0) c_mil.setPosition(m.x, m.y-time*2*TC);
                    if(m.ang == 90) c_mil.setPosition(m.x+time*2*TC, m.y);
                    if(m.ang == 180) c_mil.setPosition(m.x, m.y+time*2*TC);
                    if(m.ang == 270) c_mil.setPosition(m.x-time*2*TC, m.y);
                }
                /*
                if(passos[i][passo_atual] != ALTO)
                    c_mil.setFillColor(Color::White);
                if(passos[i][passo_atual] == ALTO)
                    c_mil.setFillColor(Color(255, 127, 127));
                */
            }

            if(number_view)
            {
                sprintf(buff, "%d", m.id);
                texto_num.setString(String(buff));
                texto_num.setPosition(m.x-texto_num.getLocalBounds().width/2.f, m.y-texto_num.getLocalBounds().height);

                /*
                if(passos[i][passo_atual] == EM_FRENTE)
                {
                    if(m.ang == 0) texto_num.setPosition(m.x-texto_num.getLocalBounds().width/2.f, m.y-texto_num.getLocalBounds().height-clock.getElapsedTime().asSeconds()*2*TC);
                    if(m.ang == 90) texto_num.setPosition(m.x-texto_num.getLocalBounds().width/2.f+clock.getElapsedTime().asSeconds()*2*TC, m.y-texto_num.getLocalBounds().height);
                    if(m.ang == 180) texto_num.setPosition(m.x-texto_num.getLocalBounds().width/2.f, m.y-texto_num.getLocalBounds().height+clock.getElapsedTime().asSeconds()*2*TC);
                    if(m.ang == 270) texto_num.setPosition(m.x-texto_num.getLocalBounds().width/2.f-clock.getElapsedTime().asSeconds()*2*TC, m.y-texto_num.getLocalBounds().height);
                }
                */
                c_mil.setTexture(nullptr);
                window.draw(c_mil);
                window.draw(texto_num);
            }
            else
            {
                if(passo_atual == 0)
                    c_mil.setTexture(&t_dois);
                else if(passos[i][passo_atual-1] == ALTO)
                {
                    c_mil.setTexture(&t_dois);
                    if(passo_atual > 1)
                        if(passos[i][passo_atual-2] == EM_FRENTE || passos[i][passo_atual-2] == MARCAR_PASSO)
                            c_mil.setFillColor(Color(255, 128, 128));
                }
                else if(passos[i][passo_atual-1] == ESQUERDA || passos[i][passo_atual-1] == MEIA_VOLTA)
                    c_mil.setTexture(&t_esq);
                else if(passos[i][passo_atual-1] == DIREITA)
                    c_mil.setTexture(&t_dir);
                else if(passo_atual % 2)
                    c_mil.setTexture(&t_esq);
                else if(passo_atual % 2 == 0)
                    c_mil.setTexture(&t_dir);
                window.draw(c_mil);
            }
        }

        /// DESENHAR RETÂNGULO DE SELEÇÃO
        if(Mouse::isButtonPressed(Mouse::Left))
        {
            r_select.setPosition(pos_down.x, pos_down.y);
            r_select.setSize(Vector2f(mouse.x-pos_down.x, mouse.y-pos_down.y));
            window.draw(r_select);
        }

        /// DESENHAR TEXTOS EFETIVO, PASSO ATUAL, TOTAL DE PASSOS
        sprintf(buff, "Efetivo: %d", militares.size());
        texto_efetivo.setString(String(buff));
        window.draw(texto_efetivo);

        sprintf(buff, "Passo atual: %d", passo_atual);
        texto_passo_atual.setString(String(buff));
        texto_passo_atual.setPosition(TC*WQ-2-texto_passo_atual.getGlobalBounds().width, 36);
        window.draw(texto_passo_atual);

        sprintf(buff, "Total de passos: %d", (passos.size() > 0)? passos[0].size() : 0);
        texto_total_passos.setString(String(buff));
        texto_total_passos.setPosition(TC*WQ-2-texto_total_passos.getGlobalBounds().width, 72);
        window.draw(texto_total_passos);


        window.display();


        /// ATUALIZAÇÃO DA ANIMAÇÃO
        if(animate)
        {
            if(clock.getElapsedTime().asSeconds() >= 0.5f)
            {
                clock.restart();
                passo_atual++;
            }
            if(passo_atual == passos[0].size()) animate=false;
        }
    }
    window.close();

    cout << "Hello world!" << endl;
    return 0;
}

vector<Militar> carregar_tropa(string filename)
{
    vector<Militar> inicial;

    fstream f;
    f.open(filename, ios::in);

    if(!f.is_open())
    {
        MessageBox(NULL, string("Não foi possível abrir o arquivo \'"+filename+"\'.\n").c_str(), "Aviso!", MB_ICONERROR);
        return vector<Militar>(0);
    }

    string linha;
    while(getline(f, linha))
    {
        stringstream ss;
        ss << linha;

        int x, y, ang, id;
        ss >> x >> y >> ang >> id;

        inicial.push_back(Militar(x, y, id, ang));
    }
    f.close();
    return inicial;
}

vector<vector<int>> carregar_passos(string filename)
{
    vector<vector<int>> passos;
    int qt_passos, qt_mil;

    fstream f;
    f.open(filename, ios::in);

    if(!f.is_open())
    {
        MessageBox(nullptr, string("Não foi possível abrir o arquivo \'"+filename+"\'.\n").c_str(), "Aviso!", MB_ICONERROR);
        return vector<vector<int>>(0);
    }

    stringstream ss;
    string linha;
    getline(f, linha);
    ss << linha;
    ss >> qt_mil >> qt_passos;
    ss.clear();

    while(getline(f, linha))
    {
        vector<int> temp;
        ss << linha;
        for(int i=0; i<qt_passos; i++)
        {
            int t;
            ss >> t;
            temp.push_back(t);
        }
        passos.push_back(temp);
        ss.clear();
    }
    f.close();
    return passos;
}

void salvar_tropa(string filename, vector<Militar> tropa)
{
    fstream f;
    f.open(filename, ios::out);

    if(!f.is_open())
    {
        MessageBox(nullptr, string("Não foi possível abrir o arquivo \'"+filename+"\'.\n").c_str(), "Aviso!", MB_ICONERROR);
        return;
    }

    for(int i=0; i<tropa.size(); i++)
        f << tropa[i].x << ' ' <<  tropa[i].y << ' ' << tropa[i].ang << ' ' << tropa[i].id << endl;
    f.close();
}

void salvar_passos(string filename, vector<vector<int>> passos)
{
    fstream f;
    f.open(filename, ios::out);

    if(!f.is_open())
    {
        MessageBox(nullptr, string("Não foi possível abrir o arquivo \'"+filename+"\'.\n").c_str(), "Aviso!", MB_ICONERROR);
        return;
    }

    if(passos.size() == 0)
    {
        MessageBox(nullptr, string("Não há militares.\n").c_str(), "Aviso!", MB_ICONERROR);
        return;
    }

    if(passos[0].size() == 0)
    {
        MessageBox(nullptr, string("Não há sequência de passos.\n").c_str(), "Aviso!", MB_ICONERROR);
        return;
    }

    f << passos.size() << ' ' << passos[0].size() << endl;
    for(int i=0; i<passos.size(); i++)
    {
        for(int j=0; j<passos[i].size(); j++)
        {
            f << passos[i][j] << ' ';
        }
        f << endl;
    }
    f.close();

    for(int i=0; i<passos.size(); i++)
    {
        char nome[10];
        sprintf(nome, "fechamento/%d.txt", i+1);

        f.open(nome, ios::out);

        int passo = passos[i][0];
        int qt=1;
        for(int j=1; j<passos[i].size(); j++)
        {
            if(passos[i][j] == passo)
                qt++;
            else
            {
                if(passos[i][j] == ALTO)
                {
                    if(passo == EM_FRENTE)
                    {
                        f << qt << " passos EM FRENTE (com alto).\n";
                    }
                    if(passo == MARCAR_PASSO)
                    {
                        f << qt << " passos MARCANDO PASSO (com alto).\n";
                    }
                    if(passo == DIREITA)
                        f << "DIREITA, VOLVER.\n";
                    if(passo == ESQUERDA)
                        f << "ESQUERDA, VOLVER.\n";
                    if(passo == MEIA_VOLTA)
                        f << "MEIA VOLTA, VOLVER.\n";
                }
                else if(passos[i][j] == DIREITA)
                {
                    if(passo == EM_FRENTE)
                        f << qt << " passos EM FRENTE (sem alto).\n" << "DIREITA VOLVER.\n";
                    if(passo == MARCAR_PASSO)
                        f << qt << " passos MARCANDO PASSO (sem alto).\n " << "DIREITA VOLVER.\n";
                }
                else if(passos[i][j] == ESQUERDA)
                {
                    if(passo == EM_FRENTE)
                        f << qt << " passos EM FRENTE (sem alto).\n" << "ESQUERDA VOLVER.\n";
                    if(passo == MARCAR_PASSO)
                        f << qt << " passos MARCANDO PASSO (sem alto).\n" << "ESQUERDA VOLVER.\n";
                }
                else if(passos[i][j] == MEIA_VOLTA)
                {
                    if(passo == EM_FRENTE)
                        f << qt << " passos EM FRENTE (sem alto).\n" << "MEIA VOLTA VOLVER.\n";
                    if(passo == MARCAR_PASSO)
                        f << qt << " passos MARCANDO PASSO (sem alto).\n" << "MEIA VOLTA VOLVER.\n";
                }
                else if(passos[i][j] == MARCAR_PASSO)
                {
                    if(passo == EM_FRENTE)
                        f << qt << " passos EM FRENTE (sem alto).\n";
                }
                else if(passos[i][j] == EM_FRENTE)
                {
                    if(passo == MARCAR_PASSO)
                        f << qt << " passos MARCANDO PASSO (sem alto).\n";
                }
                passo = passos[i][j];
                qt=1;
            }
        }
        f.close();
    }
}
