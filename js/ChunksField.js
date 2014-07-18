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

    // Method for retrieving and performing actions against staged chunks.
    this.stagedChunk = (function() {
      var stagedClaims = [];
      return {
        retrieve: function() {
          for (var d in thisField.chunks) {
            if (thisField.chunks[d].view === 'staged') {
              return thisField.chunks[d];
            }
          }
          return false;
        },
        show: function(stagedChunk, prevSibling, prevChunk) {
          var stagedRow;

          prevChunk = prevChunk || false;

          stagedChunk = this.retrieve();

          // Change view to instance_selection.
          stagedChunk.setView('instance_selection');
          stagedRow = stagedChunk.element.parents('tr.staged');
          stagedRow.removeClass('staged');
          stagedRow.insertAfter(prevSibling);
          thisField.resetWeights();
          stagedRow.show();
          thisField.setActiveChunk(stagedChunk.delta);
          thisField.resetStripes();

          // Provide a callback reacting against the showing of a staged chunk.
          for (var chunkType in Drupal.settings.chunks.callbacks.stagedChunkShown) {
            if (typeof Drupal.settings.chunks.callbacks.stagedChunkShown[chunkType] === 'function') {
              Drupal.settings.chunks.callbacks.stagedChunkShown[chunkType](stagedChunk, prevChunk);
            }
          }
        },
        claim: function(delta) {
          stagedClaims.push(delta);
        },
        redeem: function(delta) {
          var stagedChunk;

          // Return false if the requesting chunk isn't next in line.
          if (stagedClaims.length === 0 || stagedClaims[0] !== delta) {
            return false;
          }

          // Return false if there is no staged chunk to claim.
          stagedChunk = this.retrieve();
          if (stagedChunk === false) {
            return false;
          }

          // Remove the next claim.
          stagedClaims.shift();

          // Show the currently hidden staged chunk after it's redeemer.
          if (delta >= 0) {
            this.show(stagedChunk, '#' + thisField.fieldName + '-' + delta + '-chunk-row', thisField.chunks[delta]);
          }
          else {
            this.show(stagedChunk, '#' + thisField.fieldName + '-chunks-field .add-chunk-action-before-row');
          }

          // Redemption successful!
          return true;
        },
      };
    })();

    // Method for queueing actions to run one at
    this.actions = (function() {
      var actions = [];
      return {
        get: function() {
          return actions;
        },
        queue: function(label, fun) {
          actions.push({label: label, run: fun});
        },
        runNext: function() {
          if (actions.length > 0) {
            actions[0].run();
            actions.splice(0, 1);
          }
        }
      };
    })();

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

            if (Drupal.ajaxInProgress()) {
              $(this).addClass('progress-disabled').attr('disabled', true);
              thisField.actions.queue('add_before', function() {
                var button = $(':input[name="' + thisField.fieldName + '-add-before"]', thisField.element).get(0);
                // Manually request an ajax event response.
                Drupal.ajax[button.id].eventResponse(button, e);
                // Remove throbber.
                $('.staged-chunk-queued-before', thisField.element).remove();
                thisField.stagedChunk.redeem(-1);
              });
            }
            else {
              // Manually request an ajax event response.
              Drupal.ajax[this.id].eventResponse(this, e);
            }

            // Claim the next staged chunk, then see if we can immediately
            // redeem it.
            thisField.stagedChunk.claim(-1);
            if (!thisField.stagedChunk.redeem(-1)) {
              $(this).after('<div class="ajax-progress ajax-progress-throbber staged-chunk-queued-before"><div class="throbber">&nbsp;</div><div class="message">Please wait...</div></div>');
            }

            // Set all chunks to inactive so focus doesn't jump around.
            thisField.deactivateChunks();

            setTimeout(function() {
              var newChunk = thisField.activeChunk;
              $(':input[name="' + newChunk.namePrepend + '[instance]"]', newChunk.element).first().focus();
            }, 0);
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

        if (typeof chunk === 'undefined' || !thisField.settings.unlimited) {
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
              chunk.buttons.add.focus();
            }
          }
          chunk.needsReset = false;
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
