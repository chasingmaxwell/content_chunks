/**
 * @file
 * Provide the ChunksField object.
 */

(function(globals, $){

  'use strict';

  /**
   * Manage properties and methods connected to a single chunks field.
   *
   * @param {HTMLElement} element
   *   The DOM element of the chunks field to which this object is tied.
   */
  globals.ChunksField = function(element) {

    // Make it easier to reference this ChunksField when scope changes.
    var thisField = this;


    /**
     * Public properties.
     */

    this.field = $(element);
    this.fieldName = this.field.attr('field_name');
    this.classFieldName = this.field.attr('field_name').replace(/_/g, '-');
    this.langcode = this.field.attr('langcode');
    this.chunksElements = $('.chunk-wrapper', element);
    this.chunks = {};


    /**
     * Public methods.
     */

    this.setActiveChunk = function(delta) {
      for (var d in this.chunks) {
        if (parseInt(d, 10) !== delta) {
          this.chunks[d].setAsUnactive();
        }
        else {
          this.chunks[d].setAsActive();
        }
      }
    };

    this.showStagedChunk = function(prevSibling) {
      var stagedRow, stagedChunk;

      // Find the staged chunk.
      for (var d in this.chunks) {
        if (this.chunks[d].view === 'staged') {
          stagedChunk = this.chunks[d];
        }
      }

      // Change view to instance_selection.
      stagedChunk.setView('instance_selection');

      stagedRow = stagedChunk.chunk.parents('tr.staged');
      stagedRow.removeClass('staged');
      stagedRow.insertAfter(prevSibling);
      stagedRow.show();
      this.resetStripes();
    };

    // Reset odd/even striping on visible chunk rows.
    this.resetStripes = function() {
      var visibleChunks = $('.chunk-wrapper:visible', this.field);
      visibleChunks.each(function(i, element) {
        var delta, parentRow;

        delta = parseInt($(element).attr('delta'), 10);
        parentRow = $('#' + this.fieldName + '-' + delta + '-chunk-row');

        if (((i + 1) % 2) == 1) {
          parentRow.removeClass('even').addClass('odd');
        }
        else {
          parentRow.removeClass('odd').addClass('even');
        }
      });
    };


    /**
     * Register some helpers
     */

    // add here (before)
    $(':input[name="' + this.fieldName + '-add-before"]', element).bind('keyup.chunkadd mousedown.chunkadd', function(e) {
      if (e.type === 'mousedown' || e.type === 'keyup' && e.keycode === 13) {

        // show the currently hidden staged chunk above every other chunk.
        thisField.showStagedChunk('#' + thisField.fieldName + '-chunks-field .add-chunk-action-before-row');

        // set the newchunkindex to 0 so we can properly focus it when the
        // field is rebuilt.
        Drupal.settings.chunks[thisField.fieldName].newChunkIndex = 0;
      }
    });


    /**
     * Perform initial operations.
     */

    // Retrieve chunks within this field. This will be immediately run when the
    // ChunkField object is constructed and again whenever we need to refresh
    // the Chunk objects connected to it.
    this.chunksElements.each(function() {
      var delta = parseInt($(this).attr('delta'), 10);
      thisField.chunks[delta] = new Chunk(this, thisField, delta);
    });
  };

})(this, jQuery);
