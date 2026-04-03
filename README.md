# Standalone Trainer UI Preview

Небольшой `C++` пример на `Dear ImGui`, который повторяет внешний вид трейнера, но **не взаимодействует с игрой**:

- не ищет процесс;
- не читает память;
- не пишет в память;
- не отправляет инпут в игру.

## Сборка

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Зависимости

Проект использует `FetchContent` и подтягивает:

- `GLFW`
- `Dear ImGui`

## Запуск

После сборки запусти `trainer_ui` из папки `build`.
