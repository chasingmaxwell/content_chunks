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
    var thisField;

    this.setProperties = function(element) {
      thisField = this;
      this.element = $(element);
      this.fieldName = this.element.attr('field_name');
      this.classFieldName = this.element.attr('field_name').replace(/_/g, '-');
      this.settings = Drupal.settings.chunks.fields[this.fieldName];
      this.langcode = this.element.attr('langcode');
      this.chunksElements = $('.chunk-wrapper', this.element);
      this.chunks = this.chunks || {};
      this.activeChunk = this.activeChunk || null;
      this.events = this.events || [];
    };

    this.setActiveChunk = function(delta) {
      var active;
      for (var d in this.chunks) {
        active = parseInt(d, 10) === delta;
        this.chunks[d].setActiveState(active);
        if (active) {
          this.activeChunk = this.chunks[d];
        }
      }
    };

    this.deactivateChunks = function() {
      for (var d in this.chunks) {
        this.chunks[d].setActiveState(false);
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
      this.resetWeights();
      stagedRow.show();
      this.setActiveChunk(stagedChunk.delta);
      this.resetStripes();

      // Provide a callback reacting against the showing of a staged chunk.
      for (var chunkType in Drupal.settings.chunks.callbacks.stagedChunkShown) {
        if (typeof Drupal.settings.chunks.callbacks.stagedChunkShown[chunkType] === 'function') {
          Drupal.settings.chunks.callbacks.stagedChunkShown[chunkType](stagedChunk);
        }
      }
    };

    // Reset weights on chunk rows.
    this.resetWeights = function() {
      var options, total, value, rows, select, i, d, selected;

      options = {};
      total = ((this.chunksElements.length - 1) * 2) + 1;
      value = -(this.chunksElements.length - 1);

      // Create options.
      for (i = 0; i < total; i++) {
        options[i] = {
          text: value,
          value: value,
        };
        value++;
      }

      // Assign options to select elements.
      rows = $('.chunk-row', this.element);
      for (d = 0; d < rows.length; d++) {
        select = $('.' + this.fieldName + '-delta-order', rows[d])[0];
        select.options.length = 0;
        for (i in options) {
          selected = parseInt(i, 10) === d;
          select.options[i] = new Option(options[i].text, options[i].value, false, selected);
        }
      }
    };

    // Reset odd/even striping on visible chunk rows.
    this.resetStripes = function() {
      var visibleChunks = $('.chunk-wrapper:visible', this.element);
      visibleChunks.each(function(i, element) {
        var delta, parentRow;

        delta = parseInt($(element).attr('delta'), 10);
        parentRow = $('#' + thisField.fieldName + '-' + delta + '-chunk-row');

        if (((i + 1) % 2) == 1) {
          parentRow.removeClass('even').addClass('odd');
        }
        else {
          parentRow.removeClass('odd').addClass('even');
        }
      });
    };

    // Initiate event handlers.
    this.initializeEventHandlers = function() {
      var currentEvent;

      // Unbind the click event on the add before button so we can controll ajax
      // behavior manually.
      $(':input[name="' + this.fieldName + '-add-before"]', this.element).unbind('click');

      // add here (before)
      this.events.push({
        'selector': ':input[name="' + this.fieldName + '-add-before"]',
        'events': 'keyup.chunkAdd mousedown.chunkAdd',
        'handler': function(e) {
          if ((e.type === 'mousedown' && e.which === 1) || e.type === 'keyup' && e.keyCode === 13) {

            // Show a throbber if we have no staged chunk to show.
            if (thisField.settings.loadingStaged) {
              if (thisField.settings.queueNext === false) {
                $(this).after('<div class="ajax-progress ajax-progress-throbber nothing-staged"><div class="throbber">&nbsp;</div><div class="message">Please wait...</div></div>');
                thisField.settings.queueNext = 0;
              }
              return;
            }

            // show the currently hidden staged chunk above every other chunk.
            thisField.showStagedChunk('#' + thisField.fieldName + '-chunks-field .add-chunk-action-before-row');

            // Set all chunks to inactive so focus doesn't jump around.
            thisField.deactivateChunks();

            // Manually request an ajax event response.
            Drupal.ajax[this.id].eventResponse(this, e);

            setTimeout(function() {
              var newChunk = thisField.activeChunk;
              $(':input[name="' + newChunk.namePrepend + '[instance]"]', newChunk.element).first().focus();
            }, 0);

            thisField.settings.loadingStaged = true;
          }
        }
      });

      // Buttons shouldn't submit the form or make an ajax call unless we say so.
      this.events.push({
        'selector': ':input[name="' + this.fieldName + '-add-before"]',
        'events': 'click.chunksPreventDefault keydown.chunksPreventDefault',
        'handler': function(e) {
          if (e.type === 'click' || (e.type === 'keydown' && e.keyCode === 13)) {
            e.preventDefault();
          }
        }
      });

      // Unbind all old events and bind new ones.
      for (var i = 0; i < this.events.length; i++) {
        currentEvent = this.events[i];
        $(currentEvent.selector, this.element).unbind(currentEvent.events);
        $(currentEvent.selector, this.element).bind(currentEvent.events, currentEvent.handler);
      }
    };

    // Retrieve chunks contained within this field.
    this.retrieveChunks = function() {

      // Retrieve or create new Chunks for this field.
      this.chunksElements.each(function() {
        var delta, chunk;

        delta = parseInt($(this).attr('delta'), 10);
        chunk = thisField.chunks[delta];

        if (typeof chunk === 'undefined') {
          thisField.chunks[delta] = new Chunk(this, thisField, delta);
        }
        else if (chunk.needsReset && !chunk.previewLoading) {
          chunk.setProperties(this, thisField, delta);
          chunk.initializeEventHandlers();
          if (chunk.errors.length > 0) {
            chunk.setView('configuration');
            chunk.errors.first().focus();
          }
          else {
            chunk.setView(chunk.view);
            // If the user didn't interact with the form while the preview was
            // loading, the focus is set to the body when the markup is
            // inserted. Let's put them back where they were.
            if (document.activeElement.tagName === 'BODY') {
              chunk.addButton.focus();
            }
          }
          chunk.needsReset = false;
        }
        else {
          chunk.setView(chunk.view);
        }
      });
    };


    /**
     * Perform initial operations.
     */

    this.setProperties(element);
    this.initializeEventHandlers();
    this.retrieveChunks();

  };

})(this, jQuery);
