#include "Data/UndoManager.h"

void UndoManager::saveState(const Dataset& current, const std::string& description) {
    Snapshot snapshot;
    snapshot.description = description;
    snapshot.data = current;
    m_undoStack.push(snapshot);
    if (m_undoStack.size() > MAX_STATES)
        m_undoStack.removeBottom();      // forget the oldest state
    m_redoStack.clear();                 // new action -> redo history is invalid
}

bool UndoManager::undo(Dataset& current, std::string& description) {
    if (!canUndo())
        return false;
    Snapshot previous = m_undoStack.pop();

    Snapshot now;                        // remember the current state for redo
    now.description = previous.description;
    now.data = current;
    m_redoStack.push(now);

    current = previous.data;
    description = previous.description;
    return true;
}

bool UndoManager::redo(Dataset& current, std::string& description) {
    if (!canRedo())
        return false;
    Snapshot next = m_redoStack.pop();

    Snapshot now;
    now.description = next.description;
    now.data = current;
    m_undoStack.push(now);

    current = next.data;
    description = next.description;
    return true;
}

std::string UndoManager::nextUndoText() const {
    return canUndo() ? m_undoStack.top().description : std::string();
}

std::string UndoManager::nextRedoText() const {
    return canRedo() ? m_redoStack.top().description : std::string();
}

void UndoManager::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}
