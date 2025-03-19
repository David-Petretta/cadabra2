
#pragma once

#include "DataCell.hh"
#include "DocumentThread.hh"

#include <memory>

namespace cadabra {

	class DocumentThread;
	class GUIBase;

	/// \ingroup clientserver
	///
	/// All actions derive from the ActionBase object, which defines
	/// the interface they need to implement. These objects are used to
	/// pass (user) action instructions around.  They can be stored in
	/// undo/redo stacks. All actions run on the GUI thread. The
	/// update_gui members typically call members of the GUIBase class.
	/// Action objects are allowed to modify the DTree document doc,
	/// since they essentially contain code which is part of the
	/// DocumentThread object.
	///
	/// All modifications to the document are done by calling 'perform' with an
	/// action object. This enables us to implement an undo stack. This method
	/// will take care of making the actual change to the DTree document, and
	/// call back on the 'change' methods above to inform the derived class
	/// that a change has been made.


	class ActionBase {
		public:
			ActionBase(DataCell::id_t ref_id);

			/// Perform the action. This should update both the document
			/// tree data structure and the GUI. The latter is updated
			/// by calling relevant methods on the GUIBase object passed
			/// in.
			///
			/// The base class just looks up the cell given its `id_t`.
			/// If your action does not refer to a cell at all, you do
			/// not need to call the base class `execute`.

			virtual void execute(DocumentThread&, GUIBase&);

			/// Revert the change to the DTree document and the GUI.

			virtual void revert(DocumentThread&, GUIBase&)=0;

			/// Can this action be undone?
			virtual bool undoable() const;

			DataCell::id_t  ref_id;

			/// If you want a callback once this action has finished,
			/// set it here before queuing.
			std::function<void()> callback;
			
		protected:
			DTree::iterator ref;
		};

	/// \ingroup clientserver
	///
	/// Add a cell to the notebook.

	class ActionAddCell : public ActionBase {
		public:
			enum class Position { before, after, child };

			ActionAddCell(DataCell, DataCell::id_t  ref_, Position pos_);
			virtual ~ActionAddCell() {};

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

			/// Can this action be undone?
			virtual bool undoable() const override;
		private:
			// Keep track of the location where this cell is inserted into
			// the notebook.

			DataCell          newcell;
			DTree::iterator   newref;
			Position          pos;
			int               child_num;

			// If we are replacing a cell, keep track of that so we
			// report that we are not undoable.
			bool              is_replacement;

			// For input-form cells, we want no undo, as they will go
			// when the owner cell will be reverted.
			bool              is_input_form;
		};


	/// \ingroup clientserver
	///
	/// Position the cursor relative to the indicated cell. If position is 'next' and
	/// there is no input cell following the indicated one, create a new one.

	class ActionPositionCursor : public ActionBase {
		public:
			enum class Position { in, next, previous };

			ActionPositionCursor(DataCell::id_t ref_id_, Position pos_);
			virtual ~ActionPositionCursor() {};

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

		private:
			uint64_t          needed_new_cell_with_id;
			DTree::iterator   newref;
			Position          pos;
		};

	/// \ingroup clientserver
	///
	/// Update the running status of the indicated cell.

	class ActionSetRunStatus : public ActionBase {
		public:
			ActionSetRunStatus(DataCell::id_t ref_id_, bool running);
			virtual ~ActionSetRunStatus() {};

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

			virtual bool undoable() const override;
		private:
			DTree::iterator this_cell;
			bool            was_running_, new_running_;
		};

	/// \ingroup clientserver
	///
	/// Update the list of referenced variables in this cell.

	class ActionSetVariableList : public ActionBase {
		public:
			ActionSetVariableList(DataCell::id_t ref_id_, std::set<std::string>);
			virtual ~ActionSetVariableList() {};

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

			virtual bool undoable() const override;
		private:
			DTree::iterator this_cell;
			std::set<std::string> new_variables_;
		};


	/// \ingroup clientserver
	///
	/// Remove a cell and all its child cells from the document.

	class ActionRemoveCell : public ActionBase {
		public:
			ActionRemoveCell(DataCell::id_t ref_id_);
			virtual ~ActionRemoveCell();

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

		private:
			// Keep track of the location where this cell (and its child
			// cells) was in the notebook.  We keep a reference to the
			// parent cell and the index of the current cell as child of
			// that parent.

			DTree             removed_tree;
			DTree::iterator   reference_parent_cell;
			size_t            reference_child_index;
		};

	/// \ingroup clientserver
	///
	/// Replace the contents of a cell. Not undo-able.

	class ActionReplaceCell : public ActionBase {
		public:
			ActionReplaceCell(DataCell::id_t ref_id_);
			virtual ~ActionReplaceCell();

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

			virtual bool undoable() const override;
		private:
		};
	
	/// \ingroup clientserver
	///
	/// Split a cell into two separate cells, at the point of the cursor.

	class ActionSplitCell : public ActionBase {
		public:
			ActionSplitCell(DataCell::id_t ref_id);
			virtual ~ActionSplitCell();

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

		private:
			DTree::iterator newref; // the newly created cell
		};


	/// \ingroup clientserver
	///
	/// Run a cell or run all cells. 

	class ActionRunCell : public ActionBase {
		public:
			// Run a particular cell.
			ActionRunCell(DataCell::id_t ref_id);
			// Run all cells.
			ActionRunCell();
			
			virtual ~ActionRunCell();
			
			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;
			
			virtual bool undoable() const override;
		private:
			bool run_all_cells;
	};

	/// \ingroup clientserver
	///
	/// Open a notebook from a file, in the current window.

	class ActionOpen : public ActionBase {
		public:
			ActionOpen(const std::string&);
			virtual ~ActionOpen();
			
			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;
			
			virtual bool undoable() const override;
		private:
			std::string notebook_name;
	};


	/// \ingroup clientserver
	///
	/// Add a text string (can be just a single character) at the point
	/// of the cursor.
	/// This action is assumed to be triggered from a user change to
	/// the GUI cells, so will not update the GUI itself, only the
	/// underlying DTree. However, the revert method will need to
	/// update the GUI representation.

	class ActionInsertText : public ActionBase {
		public:
			ActionInsertText(DataCell::id_t ref_id, int pos, const std::string&);
			virtual ~ActionInsertText() {};

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

		private:
			DTree::iterator   this_cell;
			int         insert_pos;
			std::string text;
		};

	/// \ingroup clientserver
	///
	/// Complete text at a point in a GUI cell with one or more
	/// alternative.
	/// In contrast to ActionInsertText, this one is triggered from
	/// the server-side, so will update the GUI both for execute
	/// and revert.

	class ActionCompleteText : public ActionBase {
		public:
			ActionCompleteText(DataCell::id_t ref_id, int pos, const std::string&, int alternative);
			virtual ~ActionCompleteText() {};

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

			int length() const;
			int alternative() const;
			
		private:
			DTree::iterator   this_cell;
			int         insert_pos;
			std::string text;
			int         alternative_; // in case there is more than one completion alternative
		};

	/// \ingroup clientserver
	///
	/// Remove a text string starting at the indicated position, and
	/// with the indicated length, from the indicated cell.
	/// This action is assumed to be triggered from a user change to
	/// the GUI cells, so will not update the GUI itself, only the
	/// underlying DTree. However, the revert method will need to
	/// update the GUI representation.

	class ActionEraseText : public ActionBase {
		public:
			ActionEraseText(DataCell::id_t ref_id, int, int);
			virtual ~ActionEraseText() {};

			virtual void execute(DocumentThread&, GUIBase&) override;
			virtual void revert(DocumentThread&,  GUIBase&) override;

		private:
			DTree::iterator   this_cell;
			int from_pos, to_pos;
			std::string removed_text;
		};

	}


//
//       class ActionMergeCells



