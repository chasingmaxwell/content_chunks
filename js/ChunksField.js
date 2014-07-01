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

    this.element = $(element);
    this.fieldName = this.element.attr('field_name');
    this.classFieldName = this.element.attr('field_name').replace(/_/g, '-');
    this.langcode = this.element.attr('langcode');
    this.chunksElements = $('.chunk-wrapper', element);
    this.chunks = {};
    this.events = [];


    /**
     * Public methods.
     */

    this.setActiveChunk = function(delta) {
      var active;
      for (var d in this.chunks) {
        active = parseInt(d, 10) === delta;
        this.chunks[d].setActiveState(active);
      }
    };

    this.setActiveField = function() {
      Drupal.settings.chunks.activeField = this.fieldName;
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

      stagedRow = stagedChunk.element.parents('tr.staged');
      stagedRow.removeClass('staged');
      stagedRow.insertAfter(prevSibling);
      stagedRow.show();
      this.resetStripes();
    };

    // Reset odd/even striping on visible chunk rows.
    this.resetStripes = function() {
      var visibleChunks = $('.chunk-wrapper:visible', this.element);
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

    // Initiate event handlers.
    this.initiateEventHandlers = function() {
      var currentEvent;
      for (var i = 0; i < this.events.length; i++) {
        currentEvent = this.events[i];
        $(currentEvent.selector, this.element).bind(currentEvent.events, currentEvent.handler);
      }
    };

    // Destroy event handlers.
    this.destroyEventHandlers = function() {
      var currentEvent;
      for (var i = 0; i < this.events.length; i++) {
        currentEvent = this.events[i];
        $(currentEvent.selector, this.element).unbind(currentEvent.events);
      }

      // Destroy event handlers for chunks in the field.
      for (var delta in this.chunks) {
        this.chunks[delta].destroyEventHandlers();
      }
    };


    /**
     * Register event handlers.
     */

    // add here (before)
    this.events.push({
      'selector': ':input[name="' + this.fieldName + '-add-before"]',
      'events': 'keyup.chunkadd mousedown.chunkadd',
      'handler': function(e) {
        if (e.type === 'mousedown' || e.type === 'keyup' && e.keycode === 13) {

          // show the currently hidden staged chunk above every other chunk.
          thisField.showStagedChunk('#' + thisField.fieldName + '-chunks-field .add-chunk-action-before-row');

          // set the newchunkindex to 0 so we can properly focus it when the
          // field is rebuilt.
          Drupal.settings.chunks[thisField.fieldName].newChunkIndex = 0;
        }
      }
    });


    /**
     * Perform initial operations.
     */

    // Initiate event handlers.
    this.initiateEventHandlers();

    // Retrieve chunks within this field. This will be immediately run when the
    // ChunkField object is constructed and again whenever we need to refresh
    // the Chunk objects connected to it.
    this.chunksElements.each(function() {
      var delta = parseInt($(this).attr('delta'), 10);
      thisField.chunks[delta] = new Chunk(this, thisField, delta);
    });
  };

})(this, jQuery);
