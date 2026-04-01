#include <iostream>
#include <random>
#include <thread>
#include <array>
#include <mutex>
#include <atomic>
#include <iomanip>
#include <ctime>


// Classe TicTacToe
class TicTacToe {
  private:
  std::mutex mutex_tela; //melhorar exibição do tabuleiro usando mutex para evitar que as threads se misturem (print principalmente)
  std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
  
  /*
  //Bloco implementado deixa as variáveis como comuns.
  // A não concorrência dessas variáveis pode gerar uma leitura ERRADA de valor
  // Basicamente a situação de Data Race 
  char current_player; // Jogador atual ('X' ou 'O')
  bool game_over; // Estado do jogo
  char winner; // Vencedor do jogo
  */
  std::atomic<char> current_player; // Jogador atual ('X' ou 'O')
  std::atomic<bool> game_over; // Estado do jogo
  std::atomic<char> winner; // Vencedor do jogo

  public:
  TicTacToe() {
    // Inicializar o tabuleiro e as variáveis do jogo
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        board[i][j] = ' ';
      }
    }
    winner = '-';
    game_over = 0;
    // sorteia jogador inicial entre 'X' e 'O'
    static std::mt19937 sorteiaJogador(static_cast<unsigned int>(time(0)));
    static std::uniform_int_distribution<int> distr(0, 1);
    current_player = distr(sorteiaJogador) == 0 ? 'X' : 'O';
  }
  
  void display_board() {
    // Exibir o tabuleiro no console
    std::system("clear");
    for(int i = 0; i < 3; i++){
      std::cout<<board[i][0] << "|" << board[i][1]<< "|" << board[i][2] << std::endl;
      if(i != 2){
        std::cout << "-----" << std::endl;
      }
    }
    std::cout << std::endl;
    //melhorar a exibição do tabuleiro
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
  }
  
  bool make_move(char player, int row, int col) {
    std::lock_guard<std::mutex> lock(mutex_tela); // Garantir que apenas uma thread acesse o tabuleiro por vez
    // Implementar a lógica para realizar uma jogada no tabuleiro
    if (game_over || player != current_player) {
      return false; 
    }

    if(board[row][col] == ' '){  //casa vazia?
        board[row][col] = player; 
        game_over = is_game_over(player); //atualiza ANTES de mudar o jogador, para garantir que o vencedor seja atualizado corretamente
        
        if(player == 'O'){
          current_player = 'X';
        }else{
          current_player = 'O';
        }
        
        display_board();
        return true;
    }
    return false; //casa ocupada
  }
  
  bool check_win(char player) {
    // Verificar se o jogador atual venceu o jogo
    // linhas
    for(int i = 0; i < 3; i++){
      if(player == board[i][0] && player == board[i][1] && player == board[i][2]){
        winner = player;
        return 1;
      }
    }
    // colunas
    for(int i = 0; i < 3; i++){
      if(player == board[0][i] && player == board[1][i] && player == board[2][i]){
        winner = player;
        return 1;
      }
    }
    // diagonal
    if(player == board[0][0] && player == board[1][1] && player == board[2][2]){
      winner = player;
      return 1;
    }
    if(player == board[0][2] && player == board[1][1] && player == board[2][0]){
      winner = player;
      return 1;
    }
    return 0;
  }
  
  bool check_draw() {
    // Verificar se houve um empate
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(board[i][j] == ' '){
          return 0;
        }
      }
    }
    return 1;
  }
  
  bool is_game_over(char player) { //recebe como parâmetro o jogador que acabou de jogar, para verificar se ele venceu ou se houve empate
    // Retornar se o jogo terminou
    if(check_win(player)){
      return 1;
    }else if(check_draw()){
      winner = 'D';
      return 1;
    }else{
      winner = '-';
      return 0;
    }
  }
  
  char get_winner() {
    // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
    return winner;
  }
};

// Classe Player
class Player {
  private:
  TicTacToe& game; // Referência para a instância do jogo
  char symbol; // Símbolo do jogador ('X' ou 'O')
  std::string strategy; // Estratégia do jogador
  
  public:
  Player(TicTacToe& g, char s, std::string strat) 
  : game(g), symbol(s), strategy(strat) {}
  
  void play() {
    // Executar jogadas de acordo com a estratégia escolhida
    while(game.get_winner() == '-'){
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      if(strategy == "sequential"){
        play_sequential();
      }else{
        play_random();
      }
    }
  }
  
  private:
  void play_sequential() {
    // Implementar a estratégia sequencial de jogadas
    //checagem de fimd e tabuleiro implementada no loop da função play para evitar que o jogador fique preso em um loop infinito caso o tabuleiro esteja cheio ou o jogo já tenha acabado
    for(int i = 0; i < 3 && game.get_winner() == '-'; i++){
      for(int j = 0; j < 3 && game.get_winner() == '-'; j++){
        if(game.make_move(symbol, i, j)){
          return;
        }
      }
    }
  }
  
  void play_random() {
    // Implementar a estratégia aleatória de jogadas
    int l;
    int c;
    bool fim = 0;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distr(0, 2);
    while(!fim && game.get_winner() == '-'){ //no caso de tabuleiro cheio estava sendo travado no loop infinito, agora ele verifica se o jogo já acabou para evitar isso
      l = distr(gen);
      c = distr(gen);
      fim = game.make_move(symbol, l, c);
    } 
  }
};

// Função principal
int main() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // Inicializar o jogo e os jogadores
  TicTacToe tabuleiro;
  tabuleiro.display_board();
  Player X(tabuleiro, 'X', "sequential");
  Player O(tabuleiro, 'O', "random");
  
  // Criar as threads para os jogadores
  std::thread Jogador1(&Player::play, &X);
  std::thread Jogador2(&Player::play, &O);
  
  // Aguardar o término das threads
  Jogador1.join();
  Jogador2.join();
  
  // Exibir o resultado final do jogo
  char vencedor = tabuleiro.get_winner();
  std::cout << "====================================================" << std::endl;
 
  if(vencedor == 'D'){
    std::cout<<"O resultado é: EMPATE!\n";
  }else{
    std::cout<<"O vencedor é o jogador: "<<vencedor<<"\n";
  }
  
  std::cout << "----------------------------------------------------" << std::endl;

  std::cout << "Ohana Souza: 2023038272" << std::endl;

  // Formatação de data e hora usando manipuladores C++
  std::cout << "Executado em: " 
            << std::setfill('0') << std::setw(2) << tm.tm_mday << "/"
            << std::setfill('0') << std::setw(2) << tm.tm_mon + 1 << "/"
            << (tm.tm_year + 1900) << " "
            << std::setfill('0') << std::setw(2) << tm.tm_hour << ":"
            << std::setfill('0') << std::setw(2) << tm.tm_min << ":"
            << std::setfill('0') << std::setw(2) << tm.tm_sec << std::endl;

  std::cout << "====================================================" << std::endl;

  return 0;
}
