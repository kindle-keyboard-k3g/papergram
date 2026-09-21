# Plano de Implementação: Detecção de Bloqueio de Tela e Protetor de Tela Padrão no Kindle 3

## 1. Contexto e Problema
No Kindle Keyboard (Kindle 3 / K3), ao acionar o interruptor de energia ou após o tempo limite de inatividade, o sistema entra em modo de bloqueio/suspensão de tela. Nesse estado, os eventos de teclado e botões de navegação param de responder (ou o dispositivo entra em suspensão). No entanto, o Papergram atualmente não detecta a transição de bloqueio/tela apagada e não altera o framebuffer. Com isso, a tela permanece congelada com o último estado de conversa ou lista de mensagens, dando a impressão de travamento ou app com defeito, sem nenhum feedback visual de que o aparelho está bloqueado ("Kindle is locked").

O objetivo é:
1. **Detectar o bloqueio/suspensão do Kindle**:
   - Via evento evdev de hardware (`KEY_POWER` / `KEY_SLEEP` / `KEY_SUSPEND` no interruptor de energia, ou códigos 116 / 142 / 205).
   - Suporte a monitoramento do estado de energia Lab126 `powerd` / sysfs `/sys/power/state` ou fallback em modo host (ex: tecla configurada ou chamada de API/evento).
2. **Exibir a tela de proteção de tela padrão (Kindle Screensaver)**:
   - Uma tela dedicada (`ScreensaverScreen` ou `LockScreen`) com o visual icônico/literário clássico do Kindle (moldura e-ink, ilustração ou tipografia com textura de livro/autor e instrução clara "Kindle is locked" / "Slide power switch to unlock" ou "Deslize o interruptor para desbloquear").
   - Enquanto bloqueado, todo input de digitação fica desativado/ignorado.
3. **Restaurar perfeitamente ao desbloquear**:
   - Ao detectar o retorno (novo toque no interruptor de energia ou desbloqueio de tela), retornar exatamente para a tela anterior ativa (Login, ChatList ou Conversa).
   - Efetuar um refresh total de e-ink (`FullRefresh` / GC16) para limpar ghosting na entrada e saída do screensaver.

---

## 2. Arquitetura e Componentes

### 2.1 Detecção de Estado de Energia & Bloqueio (`hal/`)
- Adicionar códigos de tecla de energia ao enum `KeyCode`:
  - `KEY_POWER` (código Linux 116, ou códigos de slider do Kindle).
- Atualizar `input_device_evdev.cpp` para mapear `KEY_POWER` (116) e códigos de suspend.
- No fallback de desenvolvimento (`StdinInputDevice`), mapear um caractere ou atalho de teste (ex: `~` ou escape especial) para disparar `KEY_POWER`, permitindo testar no computador.

### 2.2 Tela de Proteção (`ui/screensaver_screen.h` e `.cpp`)
- Criação da classe `ScreensaverScreen : public IScreen`:
  - Herda de `IScreen` (`render`, `handleInput`, `onEnter`, `onExit`).
  - Renderiza o visual clássico do screensaver do Kindle:
    - Fundo limpo ou estampa clássica com bordas ornamentadas em e-ink monocromático/tons de cinza (8bpp/4bpp).
    - Tipografia elegante e clara centralizada:
      - Caixa central estilizada clássica do Kindle.
      - Título: *"K I N D L E"* ou logotipo com tipografia serifada pontilhada.
      - Mensagem de status: *"Device is Locked / Dispositivo Bloqueado"*.
      - Instrução no rodapé: *"Slide power switch to wake / Deslize o interruptor para reativar"*.
  - `handleInput(const InputEvent& event)`:
    - Ignora teclas alfanuméricas normais para prevenir acionamentos acidentais.
    - Responde ao `KEY_POWER` ou interruptor para despertar e acionar o desbloqueio.

### 2.3 Integração com `ScreenNavigator` e Loop Principal (`main.cpp`)
- No `ScreenNavigator`:
  - Adicionar controle de suspensão/bloqueio:
    - `void lockScreen();`
    - `void unlockScreen();`
    - `bool isLocked() const;`
    - `void setScreensaver(std::unique_ptr<IScreen> screensaver);`
  - Salva o ponteiro para a tela que estava ativa antes do bloqueio (`previous_active_screen_`).
  - Ao entrar em lock: ativa `screensaver_screen_`, chama `onEnter()`.
  - Ao sair do lock: restaura `previous_active_screen_`, chama `onEnter()`.
- No `main.cpp`:
  - Ao receber evento `KEY_POWER`:
    - Se não estiver bloqueado -> `navigator.lockScreen()`, renderiza canvas, copia para framebuffer e dispara `full_refresh` (GC16).
    - Se estiver bloqueado -> `navigator.unlockScreen()`, renderiza canvas da tela restaurada, copia para framebuffer e dispara `full_refresh` (GC16).

---

## 3. Conformidade com Object Calisthenics e Regras do Projeto
- Classes com menos de 100 linhas e métodos com menos de 15 linhas.
- Máximo de 1 nível de indentação por método.
- Sem cláusulas `else` (usar guard clauses e retornos antecipados).
- Sem alocações dinâmicas pesadas no loop de renderização.
- Testes unitários com cobertura abrangente usando o framework existente (`test_framework.h`).

---

## 4. Estratégia de Testes
1. **`test_screensaver_screen.cpp`**:
   - Testa renderização do layout e elementos visuais da tela de screensaver no `Canvas` (presença de pixels não brancos nos limites esperados do emblema e textos).
   - Testa supressão de teclas normais (letras, números, navegação).
   - Testa que tecla de power/wake desbloqueia.
2. **`test_screen_navigator.cpp`**:
   - Testa transição para `lockScreen()` e retorno para a tela anterior (`showChatList()` -> `lockScreen()` -> `unlockScreen()` restaura chat list).
3. **`test_input_evdev.cpp` / `test_fallback_devices.cpp`**:
   - Testa tradução correta de `KEY_POWER` a partir do código Linux 116 e no dispositivo fallback.
4. **Verificação Sanitizer e Debug**:
   - Executar `make test` e `make test-asan` para garantir ausência de regressões e vazamentos de memória.
