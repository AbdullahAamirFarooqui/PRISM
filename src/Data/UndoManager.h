#ifndef PRISM_UNDO_MANAGER_H
#define PRISM_UNDO_MANAGER_H

#include "Data/Dataset.h"
#include "DSA/Stack.h"
#include <string>

// ---------------------------------------------------------------------------
// UndoManager - undo / redo with TWO stacks.
//
// Before every destructive operation we push a snapshot (a copy of the
// dataset + a description) on the undo stack.
//
//   undo: current state -> redo stack,  pop undo stack -> current state
//   redo: current state -> undo stack,  pop redo stack -> current state
//
// A brand new operation clears the redo stack (you can't redo after
// changing something else).  At most MAX_STATES snapshots are kept.
// ---------------------------------------------------------------------------
class UndoManager {
public:
    void saveState(const Dataset& current, const std::string& description);

    bool undo(Dataset& current, std::string& description);
    bool redo(Dataset& current, std::string& description);

    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }
    std::string nextUndoText() const;
    std::string nextRedoText() const;
    int undoCount() const { return m_undoStack.size(); }
    int redoCount() const { return m_redoStack.size(); }

    void clear();

    static const int MAX_STATES = 20;

private:
    struct Snapshot {
        std::string description;
        Dataset data;
    };

    Stack<Snapshot> m_undoStack;
    Stack<Snapshot> m_redoStack;
};

#endif // PRISM_UNDO_MANAGER_H
