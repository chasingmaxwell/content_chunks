/**
 * @file
 * Provide the Chunk object.
 */

(function(globals, $){

  'use strict';

  /**
   * Manages properties and methods of a single chunk within a ChunkField.
   *
   * @prop {HTMLElement} element
   *   The DOM element which this Chunk ought to be tied to.
   * @prop {ChunksField} field
   *   The parent ChunkField object to which this Chunk belongs.
   * @prop {integer} delta
   *   The position of this chunk in relation to other chunks in this field.
   *   This is tied to the delta for the field value in Drupal's Field API.
   */
  globals.Chunk = function(element, field, delta) {

    var thisChunk;

    this.setProperties = function(element, field, delta) {
      thisChunk = this;
      var buttons = $('.chunk-button', element);

      this.element = $(element);
      this.field = field;
      this.delta = delta;
      this.classPrepend = '.' + field.classFieldName + '-' + delta + '-';
      this.namePrepend =  field.fieldName + '[' + field.langcode + '][' + delta + ']';
      this.instanceName = $(':input[name="' + this.namePrepend + '[instance]"]:checked').val();
      this.chunkInstance = this.field.settings.instances[this.instanceName];
      this.chunkType = $(':input[name="' + this.namePrepend + '[type]"]').val();
      this.viewElement = $(':input[name="' + this.namePrepend + '[view]"]');
      this.view = this.viewElement.val();
      this.active = $(element).hasClass('active');
      this.viewWrappers = {
        'instance_selection': $('> .form-item-' + field.classFieldName + '-' + field.langcode + '-' + this.delta + '-instance', this.element),
        'configuration': $(this.classPrepend + 'configuration', this.element),
        'preview': $(this.classPrepend + 'preview', this.element),
      };
      this.buttons = {
        reset: buttons.filter('.chunk-reset-button'),
        remove: buttons.filter('.chunk-remove-button'),
        edit: buttons.filter('.chunk-edit-button'),
        preview: buttons.filter('.chunk-preview-button'),
        cancel: buttons.filter('.chunk-cancel-button'),
        add: buttons.filter('.chunk-add-after-button'),
      };
      this.events = this.events || [];
      this.errors = $(this.classPrepend + 'configuration :input.error', this.element);
      this.needsReset = this.needsReset || false;
      this.previewLoading = this.previewLoading || false;
    };

    this.setActiveState = function(active) {
      this.active = active;
      if (active) {
        this.element.addClass('active');
      }
      else {
        this.element.removeClass('active');
      }
    };

    // Method for retrieving and performing actions against chunk configuration.
    this.config = (function() {

      var reset = function() {
        thisChunk.instanceName = $(':input[name="' + thisChunk.namePrepend + '[instance]"]:checked').val();
        thisChunk.chunkInstance = thisChunk.field.settings.instances[thisChunk.instanceName];
        thisChunk.chunkType = thisChunk.chunkInstance.type;
      };

      return {
        get: function(fromInput) {
          reset();

          var configuration = {};

          // Retrieve configuration either from user input or static settings.
          if (fromInput) {
            $('[name^="' + thisChunk.namePrepend + '[configuration][' + thisChunk.instanceName + ']"]').each(function(i, element) {
              var name = $(element).attr('name');
              var configProp = name.match(/[^\[]*(?=]$)/)[0];
              configuration[configProp] = $(element).val();
            });
          }
          else {
            configuration = thisChunk.field.settings.chunks[thisChunk.delta].configuration[thisChunk.instanceName];
          }

          return configuration;
        },
        save: function() {
          reset();

          $('[name^="' + thisChunk.namePrepend + '[configuration][' + thisChunk.instanceName + ']"]').each(function(i, element) {
            var name = $(element).attr('name');
            var configProp = name.match(/[^\[]*(?=]$)/)[0];
            thisChunk.field.settings.chunks[thisChunk.delta].configuration[thisChunk.instanceName][configProp] = $(element).val();
          });

          // Provide a callback with which to perform actions against the saved
          // configuration.
          if (typeof Drupal.settings.chunks.callbacks.saveConfig[thisChunk.chunkType] === 'function') {
            Drupal.settings.chunks.callbacks.saveConfig[thisChunk.chunkType](field.fieldName, field.langcode, thisChunk.delta);
          }
        },
        restore: function() {
          reset();

          $('[name^="' + thisChunk.namePrepend + '[configuration]"]').each(function(i, element) {
            var name = $(element).attr('name');
            var configProp = name.match(/[^\[]*(?=]$)/)[0];
            $(element).val(thisChunk.field.settings.chunks[thisChunk.delta].configuration[thisChunk.instanceName][configProp]);
          });

          // Provide a callback with which to perform actions against the
          // restored configuration.
          if (typeof Drupal.settings.chunks.callbacks.restoreConfig[thisChunk.chunkType] === 'function') {
            Drupal.settings.chunks.callbacks.restoreConfig[thisChunk.chunkType](field.fieldName, field.langcode, thisChunk.delta);
          }
        }
      };
    })();

    this.setView = function(view) {
      this.view = view;
      this.viewElement.val(view);
      this.viewElement.trigger('change');

      // Hide all conditionally visible buttons.
      this.buttons.edit.hide();
      this.buttons.preview.hide();
      this.buttons.cancel.hide();

      switch (view) {
        case 'staged':
          // Hide staged chunks from the parent row.
          this.element.parents('#' + field.fieldName + '-' + this.delta + '-chunk-row').hide();
          break;

        case 'instance_selection':
          break;

        case 'configuration':
          // Save configuration so it can be restored later if the edit is
          // cancelled.
          this.config.save();

          // Hide unused configuration forms.
          if (typeof this.instanceName !== 'undefined') {
            $('.fieldset-wrapper > div', this.viewWrappers.configuration).hide();
            $('.' + this.instanceName + '-chunk-instance-configuration', this.viewWrappers.configuration).show();
          }

          // Show preview button. The cancel button is handled by the event
          // handler for the "Edit" button to avoid showing the cancel button
          // upon instance selection.
          this.buttons.preview.show();
          break;

        case 'preview':
          // Show edit button.
          this.buttons.edit.show();
          break;
      }

      // Show wrapper element associated with the given view. Hide all others.
      for (var wrapper in this.viewWrappers) {
        if (wrapper === view) {
          this.viewWrappers[view].show();
          continue;
        }
        this.viewWrappers[wrapper].hide();
      }
    };

    // Remove active state from element by briefly removing it from the DOM.
    this.removeActiveElementState = function(el) {
      var p, s;
      p = el.parentNode;
      s = el.nextSibling;

      p.removeChild(el);
      p.insertBefore(el, s);
    };

    // Initialize event handlers.
    this.initializeEventHandlers = function() {
      var currentEvent;

      // Unbind the click event for all buttons with ajax handlers so we can
      // control that behavior manually.
      $(this.classPrepend + 'preview-button, ' + this.classPrepend + 'remove-button.unlimited, ' + this.classPrepend + 'add-after-button', this.element).unbind('click');

      // Navigate instance radio buttons.
      this.events.push({
        'selector': ':input[name="' + this.namePrepend + '[instance]"]',
        'events': 'keydown.chunkInstanceNavigate',
        'handler': function(e) {
          if ((e.keyCode === 9 && !e.shiftKey) || e.keyCode === 39 || e.keyCode === 40) {
            // Navigate down.
            var nextRadio = $(this).parent().next('.form-item').children('.chunk-instance-selection');
            if (nextRadio.length > 0) {
              e.preventDefault();
              nextRadio.focus();
            }
          }
          else if ((e.keyCode === 9 && e.shiftKey) || e.keyCode === 37 || e.keyCode === 38) {
            // Navigate up.
            var prevRadio = $(this).parent().prev('.form-item').children('.chunk-instance-selection');
            if (prevRadio.length > 0) {
              e.preventDefault();
              prevRadio.focus();
            }
          }
        }
      });

      // Instance selection.
      this.events.push({
        'selector': ':input[name="' + this.namePrepend + '[instance]"]',
        'events': 'keyup.chunkInstanceSelected click.chunkInstanceSelected',
        'handler': function(e) {
          if (e.type === 'click' || (e.type === 'keyup' && e.keyCode === 13)) {

            // Make sure the proper radio is checked and the change event has
            // fired.
            this.checked = true;
            $(this).trigger('change');

            // Update instanceName.
            thisChunk.instanceName = $(':input[name="' + thisChunk.namePrepend + '[instance]"]:checked').val();
            thisChunk.chunkType = thisChunk.field.settings.instances[thisChunk.instanceName].type;

            // Prepare form if the chunk instance is to be themed on the client.
            if (thisChunk.field.settings.instances[thisChunk.instanceName].settings.preview_on_client) {

              // Prevent default actions on preview button if we are theming on
              // the client.
              thisChunk.buttons.preview.unbind('click').unbind('keypress').bind('keypress', false);

              // Set module value since it may not be set in a form rebuild.
              $('[name^="' + thisChunk.namePrepend + '[module]"]').val(thisChunk.field.settings.instances[thisChunk.instanceName].chunk_type.module);
            }

            // Switch to configuration view.
            thisChunk.setView('configuration');

            // Set active field to prevent focus from jumping to a different chunks
            // field.
            thisChunk.field.setActiveField();

            // Add focus to first configuration item.
            setTimeout(function() {
              $('.' + thisChunk.chunkType + '-chunk-type-configuration :input:visible, [contenteditable]:visible', thisChunk.element).first().focus();
            }, 0);
          }
        }
      });

      // Preview.
      this.events.push({
        'selector': this.classPrepend + 'preview-button',
        'events': 'keyup.chunkPreview mousedown.chunkPreview',
        'handler': function(e) {
          if ((e.type === 'mousedown' && e.which === 1) || (e.type === 'keyup' && e.keyCode === 13)) {
            var preview;

            // Hide cancel button.
            thisChunk.buttons.cancel.hide();

            // Update instanceName and chunkType.
            thisChunk.instanceName = $(':input[name="' + thisChunk.namePrepend + '[instance]"]:checked').val();
            thisChunk.chunkType = thisChunk.field.settings.instances[thisChunk.instanceName].type;

            // If we should be using a client-side theme implementation,
            // prevent the ajax call and build the preview.
            if (typeof thisChunk.instanceName !== 'undefined' && thisChunk.field.settings.instances[thisChunk.instanceName].settings.preview_on_client) {

              // Save configuration data.
              thisChunk.config.save();

              // Build preview.
              preview = Drupal.theme('chunk__' + thisChunk.chunkType, thisChunk.config.get(), thisChunk.field.fieldName, thisChunk.field.langcode, thisChunk.delta);
              $(thisChunk.classPrepend + 'preview')[0].innerHTML = preview;

              // Switch to preview view.
              thisChunk.setView('preview');

            }
            else {

              // Switch to preview view.
              thisChunk.setView('preview');

              if (Drupal.ajaxInProgress()) {
                $(this).addClass('progress-disabled').attr('disabled', true);
                $(this).after('<div class="ajax-progress ajax-progress-throbber preview-queued"><div class="throbber">&nbsp;</div><div class="message">Please wait...</div></div>');
                thisChunk.field.actions.queue('preview', function() {
                  var button = thisChunk.buttons.preview.get(0);
                  // Manually request an ajax event response.
                  Drupal.ajax[button.id].eventResponse(button, e);
                  thisChunk.needsReset = true;
                  thisChunk.previewLoading = true;
                  $('.preview-queued', thisChunk.element).remove();
                });
              }
              else {
                // Manually request an ajax event response.
                Drupal.ajax[this.id].eventResponse(this, e);
                thisChunk.needsReset = true;
                thisChunk.previewLoading = true;
              }
            }

            // Set active class on the last chunk previewed.
            thisChunk.field.setActiveChunk(thisChunk.delta);
            thisChunk.field.setActiveField();

            // Set focus to add chunk button.
            setTimeout(function() {
              thisChunk.buttons.add.focus();
            }, 0);

            // Remove active state on button.
            thisChunk.removeActiveElementState(this);
          }
        }
      });

      // Edit.
      this.events.push({
        'selector': this.classPrepend + 'edit-button',
        'events': 'keyup.chunkEdit mousedown.chunkEdit',
        'handler': function(e) {
          if ((e.type === 'mousedown' && e.which === 1) || (e.type === 'keyup' && e.keyCode === 13)) {
            thisChunk.instanceName = $(':input[name="' + thisChunk.namePrepend + '[instance]"]:checked').val();
            thisChunk.chunkType = thisChunk.field.settings.instances[thisChunk.instanceName].type;

            // Switch to configuration view.
            thisChunk.setView('configuration');

            // Show cancel button.
            thisChunk.buttons.cancel.show();

            // Set active field to prevent focus from jumping to a different chunks
            // field.
            thisChunk.field.setActiveField();

            // Remove active state on button.
            thisChunk.removeActiveElementState(this);

            // Add focus to first configuration item.
            setTimeout(function() {
              $('.' + thisChunk.chunkType + '-chunk-type-configuration :input:visible, [contenteditable]:visible', thisChunk.element).first().focus();
            }, 0);
          }
        }
      });

      // Cancel.
      this.events.push({
        'selector': this.classPrepend + 'cancel-button',
        'events': 'keyup.chunkEditCancel mousedown.chunkEditCancel',
        'handler': function(e) {
          if ((e.type === 'mousedown' && e.which === 1) || (e.type === 'keyup' && e.keyCode === 13)) {

            // Hide cancel button.
            $(this).hide();

            // Switch to preview view.
            thisChunk.setView('preview');

            // Set active field to prevent focus from jumping to a different chunks
            // field.
            thisChunk.field.setActiveField();

            // Remove active state on button.
            thisChunk.removeActiveElementState(this);

            // Restore configuration.
            thisChunk.config.restore();

            // Set focus to add chunk button.
            setTimeout(function() {
              thisChunk.buttons.add.focus();
            }, 0);
          }
        }
      });

      // Remove (or Reset).
      this.events.push({
        'selector': this.classPrepend + 'remove-button',
        'events': 'keyup.chunkRemove mousedown.chunkRemove',
        'handler': function(e) {
          if ((e.type === 'mousedown' && e.which === 1) || (e.type === 'keyup' && e.keyCode === 13)) {

            // Retrieve all currently visible chunks.
            var visibleChunks = $('.chunk-wrapper:visible', thisChunk.field.field);

            // Set focus to the add after button in the chunk before this one unless
            // this is the first visible chunk in which case we should set the
            // focus to the add before button.
            visibleChunks.each(function(vi, ve) {
              if (parseInt($(ve).attr('delta'), 10) === thisChunk.delta) {
                if (vi === 0) {
                  setTimeout(function() {
                    $(':input[name="' + thisChunk.field.fieldName + '-add-before"]', thisChunk.field.field).focus();
                  }, 0);
                }
                else {
                  setTimeout(function() {
                    $(visibleChunks[vi - 1]).find('.add-chunk-action-after input').focus();
                  }, 0);
                }
            return false;
              }
            });

            // Switch to removed view.
            thisChunk.setView('removed');

            // Remove javascript settings.
            delete thisChunk.field.settings.chunks[thisChunk.delta];

            // Hide the row for the removed chunk.
            $('#' + thisChunk.field.fieldName + '-' + thisChunk.delta + '-chunk-row').hide();

            // Reset odd/even striping on whole chunks field.
            thisChunk.field.resetStripes();

            // Set active field to prevent focus from jumping to a different chunks
            // field.
            thisChunk.field.setActiveField();
          }
        }
      });

      // Reset.
      this.events.push({
        'selector': this.classPrepend + 'reset-button',
        'events': 'keyup.chunkReset mousedown.chunkReset',
        'handler': function(e) {
          if ((e.type === 'mousedown' && e.which === 1) || (e.type === 'keyup' && e.keyCode === 13)) {

            // Remove settings and Chunk object.
            delete thisChunk.field.settings.chunks[thisChunk.delta];
            delete thisChunk.field.chunks[thisChunk.delta];

            // Set active field to prevent focus from jumping to a different chunks
            // field.
            thisChunk.field.setActiveField();
          }
        }
      });

      // Add here (after)
      this.events.push({
        'selector': this.classPrepend + 'add-after-button',
        'events': 'keyup.chunkAdd mousedown.chunkAdd',
        'handler': function(e) {
          if ((e.type === 'mousedown' && e.which === 1) || (e.type === 'keyup' && e.keyCode === 13)) {

            if (Drupal.ajaxInProgress()) {
              $(this).addClass('progress-disabled').attr('disabled', true);
              thisChunk.field.actions.queue('add_after', function() {
                var button = thisChunk.buttons.add.get(0);
                // Manually request an ajax event response.
                Drupal.ajax[button.id].eventResponse(button, e);
              });
            }
            else {
              // Manually request an ajax event response.
              Drupal.ajax[this.id].eventResponse(this, e);
            }

            // Claim the next staged chunk, then see if we can immediately
            // redeem it.
            thisChunk.field.stagedChunk.claim(thisChunk.delta);
            if (!thisChunk.field.stagedChunk.redeem(thisChunk.delta)) {
              $(this).after('<div class="ajax-progress ajax-progress-throbber staged-chunk-queued"><div class="throbber">&nbsp;</div><div class="message">Please wait...</div></div>');
            }

            // Set all chunks to inactive so focus doesn't jump around.
            thisChunk.field.deactivateChunks();

            setTimeout(function() {
              var newChunk = thisChunk.field.activeChunk;
              $(':input[name="' + newChunk.namePrepend + '[instance]"]', newChunk.element).first().focus();
            }, 0);
          }
        }
      });

      // Buttons shouldn't submit the form or make an ajax call unless we say so.
      this.events.push({
        'selector': this.classPrepend + 'preview-button, ' + this.classPrepend + 'edit-button, ' + this.classPrepend + 'cancel-button, ' + this.classPrepend + 'remove-button.unlimited, ' + this.classPrepend + 'add-after-button',
        'events': 'click.chunksPreventDefault keydown.chunksPreventDefault',
        'handler': function(e) {
          if (e.type === 'click' || (e.type === 'keydown' && e.keyCode === 13)) {
            e.preventDefault();
          }
        }
      });
      for (var i = 0; i < this.events.length; i++) {
        currentEvent = this.events[i];

        // Remove old event handler.
        $(currentEvent.selector, this.element).unbind(currentEvent.events);

        // Bind the new events.
        $(currentEvent.selector, this.element).bind(currentEvent.events, currentEvent.handler);
      }
    };


    /**
     * Perform initial operations.
     */

    // Set properties.
    this.setProperties(element, field, delta);

    // Initialize event handlers.
    this.initializeEventHandlers();

    // Set initial view.
    this.setView(this.view);

    // Allow other modules to perform actions on a chunk upon it's
    // initialization.
    if (this.chunkType === '') {
      // Run the callback for every type if the chunk does not currently have
      // one assigned.
      for (var chunkType in Drupal.settings.chunks.callbacks.initialize) {
        if (typeof Drupal.settings.chunks.callbacks.initialize[chunkType] === 'function') {
          Drupal.settings.chunks.callbacks.initialize[chunkType](this);
        }
      }
    }
    else {
      // Run the callback only for the type assigned to the chunk.
      if (typeof Drupal.settings.chunks.callbacks.initialize[this.chunkType] === 'function') {
        Drupal.settings.chunks.callbacks.initialize[this.chunkType](this);
      }
    }
  };
})(this, jQuery);
