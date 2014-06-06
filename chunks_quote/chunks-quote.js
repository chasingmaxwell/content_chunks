/**
 * @file
 *   JS behavior for Quote chunk type.
 */

(function($) {

  'use strict';

  // Instantiates the Pen editor.
  Drupal.behaviors.chunksQuoteInPlace = {
    attach: function(context, settings) {

      $('.chunks-field', context).each(function(index, element) {
        var fieldName, langcode;

        fieldName = $(this).attr('field_name');
        langcode = $(this).attr('langcode');

        if (settings.chunks[fieldName].types.quote.instance_type_settings.edit_in_place && typeof Pen === 'function') {

          // @TODO: replicate pen initialization for the <cite> element.

          // Sometimes the pen-quote-menu doesn't like to go away. Let's force it.
          $('.pen-quote-menu').hide();
          $(':input', context).bind('mousedown.hidePenMenu', function() {
            $('.pen-quote-menu').hide();
          });

          // Initialize editor for the paragraph in this Quote chunk
          $('.quote-chunk p[contenteditable]').once(function() {
            var editor,
            editorConfig;

            editorConfig = {
              class: 'pen-quote',
              editor: this,
              textarea: '<textarea name="content"></textarea>', // fallback for old browsers
              list: ['bold', 'italic', 'underline', 'createlink'],
              stay: false
            };

            editor = new Pen(editorConfig);

            // Hide the regular textarea.
            $(this).parent().find('.form-textarea-wrapper').hide();

            // Update hidden textarea on keyup and blur.
            $(this).bind('keyup.chunksQuoteInPlace blur.chunksQuoteInPlace', function(e) {
              // Each keystroke, copy the data back into the form item so it gets saved
              // when the user submits the form.
              $(this).parent().find('textarea').val($(this).html());
            });

            // Prevent <div></div> tags from being added when user presses
            // "Enter".
            $(this).bind('keydown.chunksQuoteNoDiv', function(e) {
              if (e.keyCode === 13) {
                document.execCommand('insertHTML', false, '<br><br>');
                return false;
              }
            });
          });

          // Trigger blur event on all contenteditable quote chunks when the
          // pen toolbar is clicked so we save configuration correctly.
          $('.pen-quote-menu').bind('click.chunksQuoteBlur', function() {
            $('.quote-chunk p[contenteditable]').trigger('blur.chunksQuoteInPlace');
          });
        }
      });
    }
  };

  Drupal.behaviors.chunksQuoteCallbacks = {
    attach: function(context, settings) {

      // Implements the restoreConfig callback to restore saved configuration.
      if (typeof Drupal.settings.chunks.callbacks.quote.restoreConfig === 'undefined') {
        Drupal.settings.chunks.callbacks.quote.restoreConfig = function(fieldName, langcode, delta) {

          // Only do anything if this instance of the chunk type is set to be
          // edited in-place.
          if (Drupal.settings.chunks[fieldName].types.quote.instance_type_settings.edit_in_place) {
            var classFieldName, quoteConfig, configuration, configState, inPlaceEditor;

            classFieldName = fieldName.replace(/_/g, '-');
            quoteConfig = $('#' + fieldName + '-' + delta + '-chunk .quote-chunk-configuration');

            // Make a copy of the configuration and add the edit_in_place
            // property so we can render a contenteditable quote chunk
            // without changing the configuration settings for the chunk.
            configState = Drupal.settings.chunks[fieldName].chunks[delta].configuration.quote;
            configuration = {};
            for (var prop in configState) {
              configuration[prop] = configState[prop];
            }
            configuration.edit_in_place = true;

            // Generate new markup.
            inPlaceEditor = Drupal.theme.prototype.chunk__quote(configuration, fieldName, langcode, delta);

            // Insert that new markup into the editor.
            quoteConfig.find('.quote-chunk').remove();
            quoteConfig.append(inPlaceEditor);
            Drupal.behaviors.chunksQuoteInPlace.attach(quoteConfig.parents('.field-type-chunks'), Drupal.settings);
          }
        };
      }
    },
  };

  // Provide a client-side theme implementation for text chunks.
  Drupal.theme.prototype.chunk__quote = function(config, fieldName, langcode, delta) {
    var output, contentEditable;

    // Add contenteditable attribute if we're editing this chunk in-place.
    if (typeof config.edit_in_place !== 'undefined') {
      contentEditable = config.edit_in_place ? ' contenteditable' : '';
    }
    else {
      contentEditable = '';
    }

    // @TODO: replicate chunk--quote.tpl.php output.

    return output;
  };

})(jQuery);